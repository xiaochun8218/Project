#ifndef CPPTL_JSON_FOREACH_H_INCLUDED
#define CPPTL_JSON_FOREACH_H_INCLUDED

#include "value.h"

namespace Json
{
	class ForeachCallback
	{
	public:
		virtual ~ForeachCallback(){};
		virtual void OnCallback(Json::Value& Parent, Json::Value& Obj, const std::string& NameOfObj) = 0;
	};

	class Foreach
	{
	public:
		Foreach(ForeachCallback* cb);
		~Foreach();
		void ForeachObject(Json::Value& Obj);
		void ForeachArray(Json::Value& Array);

	protected:
		ForeachCallback* _Callback;
	};
}

#endif //CPPTL_JSON_FOREACH_H_INCLUDED
