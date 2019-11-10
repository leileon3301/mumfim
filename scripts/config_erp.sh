#!/bin/bash
  CC=`which mpicc`
  #CXX=/lore/mersoj/kokkos/kokkos/bin/nvcc_wrapper
  CXX=`which mpicxx`
  cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=0 \
      -DBUILD_TESTS:BOOL=ON \
      -DCMAKE_C_COMPILER=$CC \
      -DCMAKE_CXX_COMPILER=$CXX \
      -DLOGRUN=1 \
      -DENABLE_KOKKOS=OFF \
      -DENABLE_WRITE_MICRO_PER_ITER=OFF \
      -DENABLE_WRITE_MICRO_PER_STEP=OFF \
      -DCMAKE_INSTALL_PREFIX=/gpfs/u/home/PASC/PASCmrsn/scratch/erp/install/biotissue \
      -DCMAKE_PREFIX_PATH=/gpfs/u/home/PASC/PASCmrsn/scratch/erp/install/amsi/lib/cmake/amsi/ \
      -DSCOREC_DIR=/gpfs/u/home/PASC/PASCmrsn/scratch/erp/install/core/lib/cmake/SCOREC/ \
      -Dlas_DIR=/gpfs/u/home/PASC/PASCmrsn/scratch/erp/install/las/lib/cmake  \
      -Dlas_core_DIR=/gpfs/u/home/PASC/PASCmrsn/scratch/erp/install/las/lib/cmake \
      -Dyaml-cpp_DIR=/gpfs/u/home/PASC/PASCmrsn/scratch/erp/install/yaml-cpp/0.3.0/lib/pkgconfig/ \
      -DENABLE_VERBOSITY=$VERBOSITY_LEVEL \
      -DCMAKE_CXX_STANDARD=11 \
      -DCMAKE_CXX_FLAGS="-Wno-unused-variable -Wno-unused-but-set-variable" \
      /gpfs/u/home/PASC/PASCmrsn/barn/biotissue
