#pragma  once
#include <vector>
#include "function.h"

namespace datacenter
{
#define ZSKIPLIST_MAXLEVEL 64 /* Should be enough for 2^64 elements */

#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */

    /* ZSETs use a specialized version of Skiplists */
    template <typename T>
    struct zskiplistNode
    {
        T data;
        int64_t score;
        struct zskiplistNode* backward;
        struct zskiplistLevel
        {
            struct zskiplistNode* forward;
            unsigned long span;
        } level[];
    };

    template <typename T>
    struct zskiplist
    {
        struct zskiplistNode<T>* header;
        struct zskiplistNode<T>* tail;
        unsigned long length;
        int level;
    };


    /* Struct to hold a inclusive/exclusive range spec by score comparison. */
    typedef struct
    {
        int64_t min;
        int64_t max;
        int minex;
        int maxex; /* are min or max exclusive? */
    } zrangespec;

    template <typename T>
    class em_zskiplist
    {
    public:
        em_zskiplist();
        ~em_zskiplist(void);
        int zslCreate(void);
        void zslFreeNode(zskiplistNode<T>* node);
        zskiplistNode<T>* zslInsert(int64_t score, T data);
        int zslDelete(int64_t score, T data, zskiplistNode<T>** node);
        void zslDeleteNode(zskiplistNode<T>* x, zskiplistNode<T>** update);
        zskiplistNode<T>* zslUpdateScore(int64_t cur_score, T data, int64_t new_score);
        std::vector<T> getRange(int64_t start, int64_t end, uint8_t reverse, int64_t limit);
        int64_t getRange(std::vector<T>& v_data, int64_t start, int64_t end, uint8_t reverse, int64_t limit);
        void clear();
    protected:
        zskiplistNode<T>* zslCreateNode(int level, int64_t score, T data);
        zskiplistNode<T>* zslFirstInRange(zrangespec* range);
        zskiplistNode<T>* zslLastInRange(zrangespec* range);
    private:
        int zslRandomLevel(void);
        int zslValueGteMin(int64_t score, zrangespec* spec);
        int zslValueLteMax(int64_t score, zrangespec* spec);
        int zslIsInRange(zrangespec* range);
        zskiplist<T>* m_zskiplist;
    };

    template<typename T>
    em_zskiplist<T>::em_zskiplist()
    {
        m_zskiplist = nullptr;
    }

    template<typename T>
    em_zskiplist<T>::~em_zskiplist()
    {
        if (this->m_zskiplist)
        {
            zskiplistNode<T>* node = this->m_zskiplist->header->level[0].forward;
            zskiplistNode<T>* next;

            SAFEDELETE(this->m_zskiplist->header);
            while (node)
            {
                next = node->level[0].forward;
                zslFreeNode(node);
                node = next;
            }

            SAFEDELETE(this->m_zskiplist);
            m_zskiplist = nullptr;
        }
    }

    template<typename T>
    zskiplistNode<T>* em_zskiplist<T>::zslCreateNode(int level, int64_t score, T data)
    {
        zskiplistNode<T>* zn = static_cast<zskiplistNode<T>*>(malloc(sizeof(*zn) + level * (sizeof(unsigned long) + sizeof(zskiplistNode<T>*))));
        zn->score = score;
        zn->data = data;
        return zn;
    }

    template<typename T>
    void em_zskiplist<T>::clear()
    {
        if (this->m_zskiplist)
        {
            zskiplistNode<T>* node = this->m_zskiplist->header->level[0].forward;
            zskiplistNode<T>* next;

            SAFEDELETE(this->m_zskiplist->header);
            while (node)
            {
                next = node->level[0].forward;
                zslFreeNode(node);
                node = next;
            }

            SAFEDELETE(this->m_zskiplist);
            m_zskiplist = nullptr;
        }

        zslCreate();
    }

    /* Create a new skiplist. */
    template<typename T>
    int em_zskiplist<T>::zslCreate(void)
    {
        if (this->m_zskiplist)
        {
            return -1;
        }

        this->m_zskiplist = static_cast<zskiplist<T>*>(malloc(sizeof(*this->m_zskiplist)));
        this->m_zskiplist->level = 1;
        this->m_zskiplist->length = 0;
        this->m_zskiplist->header = zslCreateNode(ZSKIPLIST_MAXLEVEL, 0, nullptr);
        for (int j = 0; j < ZSKIPLIST_MAXLEVEL; ++j)
        {
            this->m_zskiplist->header->level[j].forward = nullptr;
            this->m_zskiplist->header->level[j].span = 0;
        }

        this->m_zskiplist->header->backward = nullptr;
        this->m_zskiplist->tail = nullptr;
        return 0;
    }

    /* Free the specified skiplist node. The referenced SDS string representation
     * of the element is freed too, unless node->ele is set to NULL before calling
     * this function. */
    template<typename T>
    void em_zskiplist<T>::zslFreeNode(zskiplistNode<T>* node)
    {
        SAFEDELETE(node->data);
        SAFEDELETE(node);
    }

    /* Returns a random level for the new skiplist node we are going to create.
     * The return value of this function is between 1 and ZSKIPLIST_MAXLEVEL
     * (both inclusive), with a powerlaw-alike distribution where higher
     * levels are less likely to be returned. */
    template<typename T>
    int em_zskiplist<T>::zslRandomLevel(void) {
        int level = 1;
        while ((rand() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
        {
            level += 1;
        }

        return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
    }

    /* Insert a new node in the skiplist. Assumes the element does not already
     * exist (up to the caller to enforce that). The skiplist takes ownership
     * of the passed SDS string 'ele'. */
    template<typename T>
    zskiplistNode<T>* em_zskiplist<T>::zslInsert(int64_t score, T data)
    {
        if (!this->m_zskiplist)
        {
            return nullptr;
        }

        zskiplistNode<T>* update[ZSKIPLIST_MAXLEVEL];
        unsigned int rank[ZSKIPLIST_MAXLEVEL];
        int i;
        zskiplistNode<T>* x = this->m_zskiplist->header;
        for (i = this->m_zskiplist->level - 1; i >= 0; --i)
        {
            /* store rank that is crossed to reach the insert position */
            rank[i] = i == (this->m_zskiplist->level - 1) ? 0 : rank[i + 1];
            while (x->level[i].forward &&
                x->level[i].forward->score <= score)
            {
                rank[i] += x->level[i].span;
                x = x->level[i].forward;
            }

            update[i] = x;
        }
        /* we assume the element is not already inside, since we allow duplicated
         * scores, reinserting the same element should never happen since the
         * caller of zslInsert() should test in the hash table if the element is
         * already inside or not. */
        int level = zslRandomLevel();
        if (level > this->m_zskiplist->level)
        {
            for (i = this->m_zskiplist->level; i < level; ++i)
            {
                rank[i] = 0;
                update[i] = this->m_zskiplist->header;
                update[i]->level[i].span = this->m_zskiplist->length;
            }

            this->m_zskiplist->level = level;
        }

        x = zslCreateNode(level, score, data);
        for (i = 0; i < level; ++i)
        {
            x->level[i].forward = update[i]->level[i].forward;
            update[i]->level[i].forward = x;

            /* update span covered by update[i] as x is inserted here */
            x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
            update[i]->level[i].span = (rank[0] - rank[i]) + 1;
        }

        /* increment span for untouched levels */
        for (i = level; i < this->m_zskiplist->level; ++i)
        {
            ++update[i]->level[i].span;
        }

        x->backward = (update[0] == m_zskiplist->header) ? nullptr : update[0];
        if (x->level[0].forward)
            x->level[0].forward->backward = x;
        else
            this->m_zskiplist->tail = x;

        ++this->m_zskiplist->length;
        return x;
    }

    /* Internal function used by zslDelete, zslDeleteByScore and zslDeleteByRank */
    template<typename T>
    void em_zskiplist<T>::zslDeleteNode(zskiplistNode<T>* x, zskiplistNode<T>** update)
    {
        if (!this->m_zskiplist)
        {
            return;
        }

        for (int i = 0; i < this->m_zskiplist->level; ++i)
        {
            if (update[i]->level[i].forward == x)
            {
                update[i]->level[i].span += x->level[i].span - 1;
                update[i]->level[i].forward = x->level[i].forward;
            }
            else
            {
                update[i]->level[i].span -= 1;
            }
        }

        if (x->level[0].forward)
        {
            x->level[0].forward->backward = x->backward;
        }
        else
        {
            this->m_zskiplist->tail = x->backward;
        }

        while (this->m_zskiplist->level > 1 && !this->m_zskiplist->header->level[this->m_zskiplist->level - 1].forward)
        {
            --this->m_zskiplist->level;
        }

        --this->m_zskiplist->length;
    }

    /* Delete an element with matching score/element from the skiplist.
     * The function returns 1 if the node was found and deleted, otherwise
     * 0 is returned.
     *
     * If 'node' is NULL the deleted node is freed by zslFreeNode(), otherwise
     * it is not freed (but just unlinked) and *node is set to the node pointer,
     * so that it is possible for the caller to reuse the node (including the
     * referenced SDS string at node->ele). */
    template<typename T>
    int em_zskiplist<T>::zslDelete(int64_t score, T data, zskiplistNode<T>** node)
    {
        if (NULL == this->m_zskiplist)
        {
            return -1;
        }

        zskiplistNode<T>* update[ZSKIPLIST_MAXLEVEL];
        zskiplistNode<T>* x = this->m_zskiplist->header;
        for (int i = this->m_zskiplist->level - 1; i >= 0; --i)
        {
            while (x->level[i].forward &&
                x->level[i].forward->score <= score)
            {
                x = x->level[i].forward;
            }

            update[i] = x;
        }
        /* We may have multiple elements with the same score, what we need
         * is to find the element with both the right score and object. */
        x = x->level[0].forward;
        if (x && score == x->score && x->data == data)
        {
            zslDeleteNode(x, update);
            if (!node)
                zslFreeNode(x);
            else
                *node = x;

            return 1;
        }

        return 0; /* not found */
    }

    /* Update the score of an elmenent inside the sorted set skiplist.
     * Note that the element must exist and must match 'score'.
     * This function does not update the score in the hash table side, the
     * caller should take care of it.
     *
     * Note that this function attempts to just update the node, in case after
     * the score update, the node would be exactly at the same position.
     * Otherwise the skiplist is modified by removing and re-adding a new
     * element, which is more costly.
     *
     * The function returns the updated element skiplist node pointer. */
    template<typename T>
    zskiplistNode<T>* em_zskiplist<T>::zslUpdateScore(int64_t cur_score, T data, int64_t new_score) {
        if (!this->m_zskiplist)
        {
            return nullptr;
        }

        zskiplistNode<T>* update[ZSKIPLIST_MAXLEVEL];

        /* We need to seek to element to update to start: this is useful anyway,
         * we'll have to update or remove it. */
        zskiplistNode<T>* x = this->m_zskiplist->header;
        for (int i = this->m_zskiplist->level - 1; i >= 0; --i)
        {
            while (x->level[i].forward &&
                (x->level[i].forward->score < cur_score || x->level[i].forward->score == cur_score))
            {
                x = x->level[i].forward;
            }

            update[i] = x;
        }

        /* Jump to our element: note that this function assumes that the
         * element with the matching score exists. */
        x = x->level[0].forward;
        if (!x || cur_score != x->score || x->data != data)
        {
            return nullptr;
        }

        /* If the node, after the score update, would be still exactly
         * at the same position, we can just update the score without
         * actually removing and re-inserting the element in the skiplist. */
        if ((!x->backward || x->backward->score < new_score) &&
            (!x->level[0].forward || x->level[0].forward->score > new_score))
        {
            x->score = new_score;
            return x;
        }

        /* No way to reuse the old node: we need to remove and insert a new
         * one at a different place. */
        zslDeleteNode(x, update);
        zskiplistNode<T>* new_node = zslInsert(new_score, x->data);
        /* We reused the old node x->ele SDS string, free the node now
         * since zslInsert created a new one. */
        x->data = nullptr;
        zslFreeNode(x);
        return new_node;
    }

    template<typename T>
    int em_zskiplist<T>::zslValueGteMin(const int64_t score, zrangespec* spec)
    {
        return spec->minex ? (score > spec->min) : (score >= spec->min);
    }

    template<typename T>
    int em_zskiplist<T>::zslValueLteMax(const int64_t score, zrangespec* spec)
    {
        return spec->maxex ? (score < spec->max) : (score <= spec->max);
    }

    /* Returns if there is a part of the zset is in range. */
    template<typename T>
    int em_zskiplist<T>::zslIsInRange(zrangespec* range)
    {
        /* Test for ranges that will always be empty. */
        if (range->min > range->max ||
            (range->min == range->max && (range->minex || range->maxex)))
            return 0;

        zskiplistNode<T>* x = this->m_zskiplist->tail;
        if (!x || !zslValueGteMin(x->score, range))
            return 0;

        x = this->m_zskiplist->header->level[0].forward;
        if (!x || !zslValueLteMax(x->score, range))
            return 0;

        return 1;
    }

    /* Find the first node that is contained in the specified range.
     * Returns NULL when no element is contained in the range. */
    template<typename T>
    zskiplistNode<T>* em_zskiplist<T>::zslFirstInRange(zrangespec* range)
    {
        if (!this->m_zskiplist)
        {
            return nullptr;
        }

        /* If everything is out of range, return early. */
        if (!zslIsInRange(range))
            return nullptr;

        zskiplistNode<T>* x = this->m_zskiplist->header;
        for (int i = this->m_zskiplist->level - 1; i >= 0; --i)
        {
            /* Go forward while *OUT* of range. */
            while (x->level[i].forward && !zslValueGteMin(x->level[i].forward->score, range))
                x = x->level[i].forward;
        }

        /* This is an inner range, so the next node cannot be NULL. */
        x = x->level[0].forward;
        if (!x)
        {
            return nullptr;
        }

        /* Check if score <= max. */
        if (!zslValueLteMax(x->score, range))
            return nullptr;

        return x;
    }

    /* Find the last node that is contained in the specified range.
     * Returns NULL when no element is contained in the range. */
    template<typename T>
    zskiplistNode<T>* em_zskiplist<T>::zslLastInRange(zrangespec* range)
    {
        if (!this->m_zskiplist)
        {
            return nullptr;
        }

        /* If everything is out of range, return early. */
        if (!zslIsInRange(range))
            return nullptr;

        zskiplistNode<T>* x = this->m_zskiplist->header;
        for (int i = this->m_zskiplist->level - 1; i >= 0; --i)
        {
            /* Go forward while *IN* range. */
            while (x->level[i].forward && zslValueLteMax(x->level[i].forward->score, range))
                x = x->level[i].forward;
        }

        /* This is an inner range, so this node cannot be NULL. */
        if (!x)
        {
            return nullptr;
        }

        /* Check if score >= min. */
        if (!zslValueGteMin(x->score, range))
            return nullptr;

        return x;
    }

    template<typename T>
    std::vector<T>em_zskiplist<T>::getRange(const int64_t start, const int64_t end, uint8_t reverse, int64_t limit)
    {
        std::vector<T> v_sds;
        this->getRange(v_sds, start, end, reverse, limit);
        return v_sds;
    }

    template<typename T>
    int64_t em_zskiplist<T>::getRange(std::vector<T>& v_data, int64_t start, int64_t end, uint8_t reverse, int64_t limit)
    {
        if (limit <= 0)
        {
            limit = -1;
        }

        zrangespec range = { 0 };
        range.min = start;
        range.max = end;
        zskiplistNode<T>* ln = reverse
            ? zslLastInRange(&range)
            : zslFirstInRange(&range);

        /* No "first" element in the specified interval. */
        if (!ln)
        {
            return 0;
        }

        long offset = 0;
        /* If there is an offset, just traverse the number of elements without
         * checking the score because that is done in the next loop. */
        while (ln && offset--)
        {
            ln = reverse
                ? ln->backward
                : ln->level[0].forward;
        }

        int64_t record_count = 0;
        while (ln && limit--)
        {
            /* Abort when the node is no longer in range. */
            if (reverse)
            {
                if (!zslValueGteMin(ln->score, &range))
                    break;
            }
            else
            {
                if (!zslValueLteMax(ln->score, &range))
                    break;
            }

            v_data.push_back(ln->data);
            ++record_count;
            /* Move to next node */
            ln = reverse
                ? ln->backward
                : ln->level[0].forward;
        }

        return record_count;
    }
}