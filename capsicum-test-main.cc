#include <sys/types.h>
#ifdef __linux__
#include <sys/vfs.h>
#include <linux/magic.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <iostream>
#include "gtest/gtest.h"
#include "capsicum-test.h"

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  for (int ii = 1; ii < argc; ii++) {
    if (strcmp(argv[ii], "-v") == 0) {
      verbose = true;
    } else if (strcmp(argv[ii], "-T") == 0) {
      ii++;
      assert(ii < argc);
      tmpdir = argv[ii];
      struct stat info;
      stat(tmpdir, &info);
      assert(S_ISDIR(info.st_mode));
    } else if (strcmp(argv[ii], "-t") == 0) {
      force_mt = true;
    } else if (strcmp(argv[ii], "-F") == 0) {
      force_nofork = true;
    } else if (strcmp(argv[ii], "-u") == 0) {
      if (++ii >= argc) {
        std::cerr << "-u needs argument" << std::endl;
        exit(1);
      }
      if (isdigit(argv[ii][0])) {
        other_uid = atoi(argv[ii]);
      } else {
        struct passwd *p = getpwnam(argv[ii]);
        if (!p) {
          std::cerr << "Failed to get entry for " << argv[ii] << ", errno=" << errno << std::endl;
          exit(1);
        }
        other_uid = p->pw_uid;
      }
    }
  }
  if (other_uid == 0) {
    struct stat info;
    if (stat(argv[0], &info) == 0) {
      other_uid = info.st_uid;
    }
  }

#ifdef __linux__
  // Check whether our temporary directory is on a tmpfs volume.
  struct statfs fsinfo;
  statfs(tmpdir, &fsinfo);
  tmpdir_on_tmpfs = (fsinfo.f_type == TMPFS_MAGIC);
#endif

  int rc = RUN_ALL_TESTS();
  ShowSkippedTests(std::cerr);
  return rc;
}
