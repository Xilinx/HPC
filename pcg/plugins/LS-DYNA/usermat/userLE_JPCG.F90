! This userLE works only in serial (SMP, or MPP with 1 MPI rank).
! It wraps the call to the C version of userLE_JPCG, for Xilinx.
!
! R. F. Lucas, 210417.
!
#define POINTER_CONTIGUOUS pointer,contiguous
module userLE
  real*8, allocatable, save :: fake(:)
contains
!
!=======================================================================
!
!  User Supplied LE Solver
!
  subroutine userLESolve ( world,   & ! MPI world (null in SMP)
                           option,  & ! option ignored for Jacobi PCG
                           niq,     & ! number of columns on this process
                           indIq,   & ! list of columns on this process
                           nzlK,    & ! number of nonzeroes in matrix on this process
                           colptrK, & ! integer array of length niq+1 pointing into
                                      ! arrays rowindK and valsK to the start of each column
                           rowindK, & ! integer array of length nzlK holding row indices
                           valsK,   & ! real*8  array of length nzlK holding matrix values
                           msglvl,  & ! message level for output
                           msgunit, & ! unit number of output
                           icntl,   & ! integer control coming from field 3 of the second
                                      ! line of *CONTROL_IMPLICIT_SOLVER
                           rcntl,   & ! real*8 array of length 5 holding the numbers from fields
                                      ! 4 through 8 of the second line of *CONTROL_IMPLICIT_SOLVER
                           nrhs,    & ! number of right hand sides
                           valsB,   & ! real*8 array dimensioned niq by nrhs holding the rhs values
                                      ! overwritten by the solution vectors on output for option > 1
                           irc )      ! return code
                                      ! = 0 normal termination
                                      ! > 0 warning but LSDYNA will continue
                                      ! < 0 fatal error, will terminate
!
!******************************************************************
!|  Livermore Software Technology Corporation  (LSTC)             |
!|  ------------------------------------------------------------  |
!|  Copyright 1987-2008 Livermore Software Tech. Corp.            |
!|  All rights reserved                                           |
!******************************************************************
!
!  top level subroutine for user supplied linear equation solver
!
!----------------------------------------------------------------------
!
  implicit none
  include 'mpif.h'
!
!----------------------------------------------------------------------
!
! input parameters
!
  integer world, option, niq, nzlK, msglvl, msgunit, icntl(*), nrhs, irc
!
  integer indIq(niq), colptrK(niq+1), rowindK(nzlK)
!
  real*8  valsK(nzlK), rcntl(*), valsB(*)
!
! local variables
  integer i, j, k
  integer npes, maxit,  niter, error
  real*8  tol,  relres, flops, nrm2,  nrmB
  real*8, POINTER_CONTIGUOUS :: valsX(:), valsY(:), dprec(:)
  character*24 operation
!
! wall clock timing
  real*8  wtime0, wtime1
!
! Identify the first call, to trigger acquiring the accelerator.
  logical first_call
  save    first_call
  data    first_call /.true./
!
  integer*8 jpcg_handle, device_id
!-------------------------------------------------------------------------------
!
  irc = 0
  call wallclock(wtime0)
 
#ifdef AUTODOUBLE
  if ( msglvl .ge. 2 ) write(msgunit,'(''entering userLESolve'', i8)') option
#else
  write(      6,'(''userLE_JPCG only works with autodouble'')')
  write(msgunit,'(''userLE_JPCG only works with autodouble'')')
  call adios(TC_ERROR)
#endif
 
! Sanity check for MPP

  if (world .ne. MPI_COMM_NULL) then
    call mf2CommSize (world, npes, error)
    if (error .ne. 0) go to 9000

    if (npes .ne. 1) then
      write(msgunit,'(''userLE Jacobi PCG MPP error'', i8)') npes
      go to 9000
    end if
  end if

! If option < 1, then matrix structure has changed.
  if (first_call) then
    write(msgunit,'(''userLESolve load FPGA circuit'')')
    first_call = .false.
    option     = 0
    device_id = 1
    call userLE_JPCG_create_handle (jpcg_handle, device_id, '../cgSolver.xclbin')
  end if

! If option < 1, then matrix structure has changed.

  if ( option .eq. 0 ) then
    write(msgunit,'(''userLESolve update structure'')')
    option = 2
    call userLE_JPCG_set_matrix(jpcg_handle, niq, nzlk, colptrK, rowindK, valsK)
  end if

! If option < 3, then matrix has changed in value, and must be reloaded.

  if ( option .ne. 3 ) then
    write(msgunit,'(''userLESolve update values'')')
  end if

!-------------------------------------------------------------------------------
!
! Solve Phase
 
  operation = 'userLE_JPCG'
  if ( msglvl .ge. 1 ) call wallclock(wtime0)

  allocate (valsX(niq), stat = error)
  if (error .ne. 0) go to 9000
  allocate (dprec(niq), stat = error)
  if (error .ne. 0) go to 9000

! Form the Jacobi preconditioner

  do i = 1, niq
    dprec(i) = 1.0
  end do

  do j = 1, niq
    do i = colptrK(j), colptrK(j + 1) - 1
      if (rowindK(i) .eq. j) then
        dprec(j) = valsK(i)
      end if
    end do
  end do

! Get iteration control parameters

  maxit = 10000 ! icntl(5)
  tol   = 1e-16  ! rcntl(1)

#if 1
  call userLE_JPCG (jpcg_handle, niq,   nzlK,   colptrK, rowindK, valsK, &
                    dprec, maxit,  tol,     valsB,   valsX, &
                    niter, relres, flops )
#else
  irc = - 1
#endif
  if ( irc .lt. 0 ) go to 9000

  if ( msglvl .ge. 1 ) then
    write(msgunit,'(''userLE Jacobi PCG iteration :'', 2i12)')     niter,  maxit
    write(msgunit,'(''userLE Jacobi PCG residual  :'', 1P2E12.4)') relres, tol
    write(msgunit,'(''userLE Jacobi PCG operations:'', 1P2E12.4)') flops
  end if

#if 1
!
! Double check the result
!
  allocate (valsY(niq), stat = error)
  if (error .ne. 0) go to 9000

  do i = 1, niq
    valsY(i) = 0.0
  end do

  nrm2 = 0.0
  nrmB = 0.0
  do j = 1, niq
    do k = colptrK(j), colptrK(j + 1) - 1
      i = rowindK(k)
      valsY(i) = valsY(i) + valsK(k) * valsX(j)
      if ( i .ne. j ) valsY(j) = valsY(j) + valsK(k) * valsX(i)
    end do
  end do

  do i = 1, niq
    valsY(i) = valsB(i) - valsY(i)
  end do

  nrm2 = 0.0
  nrmB = 0.0
  do i = 1, niq
    nrm2 = nrm2 + valsY(i) * valsY(i)
    nrmB = nrmB + valsB(i) * valsB(i)
  end do
  nrm2 = sqrt(nrm2) / sqrt(nrmB)

  write(msgunit,'(''two norm of the residual    :'', 1P1E12.4)') nrm2

  deallocate (valsY)
#endif

!
! Overwrite the initial RHS with the solution
!
  do i = 1, niq
    valsB(i) = valsX(i)
  end do

  deallocate (valsX)
  deallocate (dprec)

!
!-------------------------------------------------------------------------------
!
  if ( msglvl .ge. 2 ) then
    call wallclock(wtime1)
    if (wtime1 .gt. wtime0) then
      write(msgunit,'(''WCT & GFlop/s: UserLE Solution'', f11.3, f12.3)') &
            (wtime1-wtime0), ((flops / (wtime1-wtime0)) / 10**9)
    else
      write(msgunit,'(''WCT: UserLE Solution was zero'')')
    end if
  end if
!
  if ( msglvl .ge. 2 ) write(msgunit,'(''Leaving UserLESolve after Solve'')')
!
  return
!
!-------------------------------------------------------------------------------
!
! Error Trap
!
9000 write(msgunit,9001) operation, irc
9001 format('*** Fatal Error in userLESolve'    &
           /'  operation ', a24                 &
           /'  error code = ',i8 )
!
!-------------------------------------------------------------------------------
!
  end subroutine
!
!=======================================================================
!
  subroutine userLEClean ( world, irc )
!
! clean up persistent data for userLESolve stored in module userLE
!
  implicit none
!
  integer  world, irc
!
  if ( allocated(fake) ) deallocate(fake)
  irc = 0
!
  return
  end subroutine
!
!=======================================================================
end module


