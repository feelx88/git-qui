#ifndef ACTIONTAG_H
#define ACTIONTAG_H

/**
 * @brief enum for tagging action signals from GitInterface.
 */
enum ActionTag {
  NO_TAG = 0,
  STDERR,
  GIT_STATUS,
  GIT_LOG,
  GIT_FETCH,
  GIT_COMMIT,
  GIT_ADD,
  GIT_REMOVE,
  GIT_DIFF,
  GIT_PUSH,
  GIT_PULL,
  GIT_RESET,
  GIT_BRANCH,
  GIT_STASH
};

#endif // ACTIONTAG_H
