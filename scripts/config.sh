#!/bin/bash
# Cmake config for bio
# usage ./config.sh [build_type] [logrun_flag]
source $DEVROOT/scripts/util
ROOT=`dirname $PWD`
LOGRUN_OVERRIDE=$2
if [ -z $1 ]; then
  BUILD_TYPE=Debug
else
  BUILD_TYPE=$1
fi
if [ "$BUILD_TYPE" == "Debug" ] ; then
  BUILD_DIR=$ROOT/build_debug
elif [ "$BUILD_TYPE" == "Release" ] ; then
  BUILD_DIR=$ROOT/build_release
fi
LOGRUN=ON
if [ "$LOGRUN_OVERRIDE" != "" ] ; then
  LOGRUN=$LOGRUN_OVERRIDE
fi
verify_directory_recreate ${BUILD_DIR}
cd $BUILD_DIR
module load cmake
HOSTNAME=`hostname`
if [ "$HOSTNAME" == "q.ccni.rpi.edu" ]; then
  cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER="mpicc" \
    -DCMAKE_CXX_COMPILER="mpicxx" \
    -DCMAKE_INSTALL_PREFIX=/gpfs/u/scratch/PASC/shared/install/bio/ \
    -DBUILD_TESTS=OFF \
    -DLOGRUN=$LOGRUN \
    -DCMAKE_PREFIX_PATH=/gpfs/u/scratch/PASC/shared/install/amsi/lib/cmake/amsi \
    -DSCOREC_DIR=/gpfs/u/scratch/PASC/shared/install/core/lib/cmake/SCOREC \
    -DSPARSKIT_DIR=/gpfs/u/scratch/PASC/shared/install/sparskit \
    ..
  chmod g+rw $BUILD_DIR
else
  CC=`which mpicc`
  CXX=`which mpicxx`
  cmake \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -DBUILD_TESTS=ON \
      -DCMAKE_C_COMPILER=$CC \
      -DCMAKE_CXX_COMPILER=$CXX \
      -DLOGRUN=1 \
      -DWRITE_MICRO_ITER=0 \
      -DWRITE_MICRO_STEP=0 \
      -DCMAKE_INSTALL_PREFIX=$DEVROOT/install/bio/ \
      -DCMAKE_PREFIX_PATH=$DEVROOT/install/amsi/lib/cmake/amsi \
      -DSCOREC_DIR=$DEVROOT/install/core/lib/cmake/SCOREC \
      -Dlas_DIR=$DEVROOT/install/las/lib/cmake \
      -Dlas_core_DIR=$DEVROOT/install/las/lib/cmake \
      -DMEMORYCHECK_SUPPRESSIONS_FILE=$DEVROOT/install/openmpi/1.10.7/share/openmpi/openmpi-valgrind.supp \
      ..
fi



