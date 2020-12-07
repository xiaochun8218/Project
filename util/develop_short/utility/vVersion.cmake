#vVersion.cmake

#vDateTime
string(TIMESTAMP vDateTime "%Y-%m-%d %H:%M:%S")

#vProjectDir
execute_process(
  COMMAND basename ${CMAKE_CURRENT_SOURCE_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE vProjectDir
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#vGitBranch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE vGitBranch
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#vGitCommit
execute_process(
  COMMAND git log -1 --pretty=format:%H
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE vGitCommit
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions( -DPROJECT_NAME=\"${CMAKE_PROJECT_NAME}\")
add_definitions( -DP_VERSION=\"${PROJECT_VERSION}\")
add_definitions( -DGIT_REVISION=\"${vGitCommit}\")
add_definitions( -DGIT_BRANCH=\"${vGitBranch}\")
add_definitions( -DRELEASE_DATETIME=\"${vDateTime}\")
