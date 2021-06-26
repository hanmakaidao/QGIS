# Find ColMap
# ~~~~~~~~~~
# Copyright (c) 2020, Peter Petrik <zilolv at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for ColMap library
#
# If it's found it sets ColMap_FOUND to TRUE
# and following variables are set:
#    ColMap_INCLUDE_DIR
#    ColMap_LIBRARIES

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing.

FIND_PATH(ColMap_INCLUDE_DIR base/camera.h
  "$ENV{LIB_DIR}/include"
  "/usr/include"
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(ColMap_INCLUDE_DIR base/camera.h)

FIND_LIBRARY(ColMap_CPP_LIBRARY NAMES colmap  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_CPP_LIBRARY NAMES ColMap libColMap)

FIND_LIBRARY(ColMap_CUDA_LIBRARY NAMES colmap_cuda  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_CUDA_LIBRARY NAMES ColMapCUDA libColMapCUDA)


FIND_LIBRARY(ColMap_Flann_LIBRARY NAMES flann  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_flann_LIBRARY NAMES ColMapflann libColMapflann)

FIND_LIBRARY(ColMap_graclus_LIBRARY NAMES graclus  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_graclus_LIBRARY NAMES ColMapgraclus libColMapgraclus)

FIND_LIBRARY(ColMap_lsd_LIBRARY NAMES lsd  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_lsd_LIBRARY NAMES ColMaplsd libColMaplsd)

FIND_LIBRARY(ColMap_pba_LIBRARY NAMES pba  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_pba_LIBRARY NAMES ColMappba libColMappba)

FIND_LIBRARY(ColMap_poisson_LIBRARY NAMES poisson_recon  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_poisson_LIBRARY NAMES ColMappoisson libColMappoisson)

FIND_LIBRARY(ColMap_siftgpu_LIBRARY NAMES sift_gpu  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_siftgpu_LIBRARY NAMES ColMapsiftgpu libColMapsiftgpu)

FIND_LIBRARY(ColMap_sqlite3_LIBRARY NAMES sqlite3  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_sqlite3_LIBRARY NAMES ColMapssqlite3 libColMapsqlite3)

FIND_LIBRARY(ColMap_vlfeat_LIBRARY NAMES vlfeat  PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(ColMap_vlfeat_LIBRARY NAMES ColMapvlfeat libColMapvlfeat)

FIND_PROGRAM(ColMap_BIN ColMap
    $ENV{LIB_DIR}/bin
    /usr/local/bin/
    /usr/bin/
    NO_DEFAULT_PATH
    )
FIND_PROGRAM(ColMap_BIN ColMap)

IF (ColMap_INCLUDE_DIR AND ColMap_CPP_LIBRARY )
   SET(ColMap_FOUND TRUE)
   SET(ColMap_LIBRARIES ${ColMap_CPP_LIBRARY} ${ColMap_CUDA_LIBRARY})
ENDIF (ColMap_INCLUDE_DIR AND ColMap_CPP_LIBRARY )

