#!/usr/bin/bash

module load impi/19.0.9

export GASNET_PHYSMEM_MAX='167 GB'
export I_MPI_PMI_LIBRARY=/opt/apps/pmix/3.2.3/lib/libpmi2.so
