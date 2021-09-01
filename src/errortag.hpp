#ifndef ERRORTAG_HPP
#define ERRORTAG_HPP

/**
 * @brief enum for tagging error signals from GitInterface.
 */
enum ErrorTag {
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

#endif // ERRORTAG_HPP
