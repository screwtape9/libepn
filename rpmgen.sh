#!/bin/sh

read -p "Create library source tarball? (Y/N) " ANSWER
if [ "$ANSWER" == "Y" -o "$ANSWER" == "y" ]; then
  MAJ=`grep MAJOR_VERSION makefile.common | awk 'BEGIN { FS="=" }; { print $2 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  MIN=`grep MINOR_VERSION makefile.common | awk 'BEGIN { FS="=" }; { print $2 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  PAT=`grep PATCH_LEVEL makefile.common | awk 'BEGIN { FS="=" }; { print $2 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`

  RMAJ=`grep '%define major' libepn.spec | awk '{ print $3 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  RMIN=`grep '%define minor' libepn.spec | awk '{ print $3 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  RPAT=`grep '%define patchlevel' libepn.spec | awk '{ print $3 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`

  [ "$MAJ" -ne "$RMAJ" ] && echo "Library major version doesn't match" && exit 1
  [ "$MIN" -ne "$RMIN" ] && echo "Library minor version doesn't match" && exit 1
  [ "$PAT" -ne "$RPAT" ] && echo "Library patch level doesn't match" && exit 1

  make clean
  rm -rf libepn-$MAJ.$MIN.$PAT
  mkdir libepn-$MAJ.$MIN.$PAT
  cp -R include/ lib/ makefile.common mkinstalldirs src/ libepn-$MAJ.$MIN.$PAT
  tar czf libepn-$MAJ.$MIN.$PAT.tar.gz libepn-$MAJ.$MIN.$PAT
  rm -rf libepn-$MAJ.$MIN.$PAT

  read -p "Copy tarball and spec file to ~/rpmbuild directory? (Y/N) " ANSWER
  if [ "$ANSWER" == "Y" -o "$ANSWER" == "y" ]; then
    cp libepn-$MAJ.$MIN.$PAT.tar.gz ~/rpmbuild/SOURCES
    cp libepn.spec ~/rpmbuild/SPECS
    rm libepn-$MAJ.$MIN.$PAT.tar.gz
  fi

  echo "Giddy up!"
fi

read -p "Create devel source tarball? (Y/N) " ANSWER
if [ "$ANSWER" == "Y" -o "$ANSWER" == "y" ]; then
  MAJ=`grep MAJOR_VERSION makefile.common | awk 'BEGIN { FS="=" }; { print $2 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  MIN=`grep MINOR_VERSION makefile.common | awk 'BEGIN { FS="=" }; { print $2 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  PAT=`grep PATCH_LEVEL makefile.common | awk 'BEGIN { FS="=" }; { print $2 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`

  RMAJ=`grep '%define major' libepn-devel.spec | awk '{ print $3 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  RMIN=`grep '%define minor' libepn-devel.spec | awk '{ print $3 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`
  RPAT=`grep '%define patchlevel' libepn-devel.spec | awk '{ print $3 }' | awk '{ gsub(/^[ \t]+|[ \t]+$/, ""); print }'`

  [ "$MAJ" -ne "$RMAJ" ] && echo "Library major version doesn't match" && exit 1
  [ "$MIN" -ne "$RMIN" ] && echo "Library minor version doesn't match" && exit 1
  [ "$PAT" -ne "$RPAT" ] && echo "Library patch level doesn't match" && exit 1

  make clean
  rm -rf libepn-devel-$MAJ.$MIN.$PAT
  mkdir libepn-devel-$MAJ.$MIN.$PAT
  cp -R include/ lib/ makefile.common mkinstalldirs src/ libepn-devel-$MAJ.$MIN.$PAT
  tar czf libepn-devel-$MAJ.$MIN.$PAT.tar.gz libepn-devel-$MAJ.$MIN.$PAT
  rm -rf libepn-devel-$MAJ.$MIN.$PAT

  read -p "Copy tarball and spec file to ~/rpmbuild directory? (Y/N) " ANSWER
  if [ "$ANSWER" == "Y" -o "$ANSWER" == "y" ]; then
    cp libepn-devel-$MAJ.$MIN.$PAT.tar.gz ~/rpmbuild/SOURCES
    cp libepn-devel.spec ~/rpmbuild/SPECS
    rm libepn-devel-$MAJ.$MIN.$PAT.tar.gz
  fi

  echo "Giddy up!"
fi
