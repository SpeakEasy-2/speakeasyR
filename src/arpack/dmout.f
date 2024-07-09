*-----------------------------------------------------------------------
*  Routine:    DMOUT
*
*  Purpose:    Real matrix output routine.
*
*  Usage:      CALL DMOUT (LOUT, M, N, A, LDA, IDIGIT, IFMT)
*
*  Arguments
*     M      - Number of rows of A.  (Input)
*     N      - Number of columns of A.  (Input)
*     A      - Real M by N matrix to be printed.  (Input)
*     LDA    - Leading dimension of A exactly as specified in the
*              dimension statement of the calling program.  (Input)
*     IFMT   - Format to be used in printing matrix A.  (Input)
*     IDIGIT - Print up to IABS(IDIGIT) decimal digits per number.  (In)
*              If IDIGIT .LT. 0, printing is done with 72 columns.
*              If IDIGIT .GT. 0, printing is done with 132 columns.
*
*-----------------------------------------------------------------------
*
      SUBROUTINE DMOUT( LOUT, M, N, A, LDA, IDIGIT, IFMT )
*     ...
*     ... SPECIFICATIONS FOR ARGUMENTS
*     ...
*     ... SPECIFICATIONS FOR LOCAL VARIABLES
*     .. Scalar Arguments ..
      CHARACTER*( * )    IFMT
      INTEGER            IDIGIT, LDA, LOUT, M, N
*     ..
*     .. Array Arguments ..
      DOUBLE PRECISION   A( LDA, * )
*     ..
*     .. Local Scalars ..
      CHARACTER*80       LINE
      INTEGER            I, J, K1, K2, LLL, NDIGIT
*     ..
*     .. Local Arrays ..
      CHARACTER          ICOL( 3 )
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          LEN, MIN, MIN0
*     ..
*     .. Data statements ..
      DATA               ICOL( 1 ), ICOL( 2 ), ICOL( 3 ) / 'C', 'o',
     $                   'l' /
*     ..
*     .. Executable Statements ..
*     ...
*     ... FIRST EXECUTABLE STATEMENT
*
      LLL = MIN( LEN( IFMT ), 80 )
      DO 10 I = 1, LLL
         LINE( I: I ) = '-'
   10 CONTINUE
*
      DO 20 I = LLL + 1, 80
         LINE( I: I ) = ' '
   20 CONTINUE
*
*
      IF( M.LE.0 .OR. N.LE.0 .OR. LDA.LE.0 )
     $   RETURN
      NDIGIT = IDIGIT
      IF( IDIGIT.EQ.0 )
     $   NDIGIT = 4
*
*=======================================================================
*             CODE FOR OUTPUT USING 72 COLUMNS FORMAT
*=======================================================================
*
      IF( IDIGIT.LT.0 ) THEN
         NDIGIT = -IDIGIT
         IF( NDIGIT.LE.4 ) THEN
            DO 40 K1 = 1, N, 5
               K2 = MIN0( N, K1+4 )
   40       CONTINUE
*
         ELSE IF( NDIGIT.LE.6 ) THEN
            DO 60 K1 = 1, N, 4
               K2 = MIN0( N, K1+3 )
   60       CONTINUE
*
         ELSE IF( NDIGIT.LE.10 ) THEN
            DO 80 K1 = 1, N, 3
               K2 = MIN0( N, K1+2 )
   80       CONTINUE
*
         ELSE
            DO 100 K1 = 1, N, 2
               K2 = MIN0( N, K1+1 )
  100       CONTINUE
         END IF
*
*=======================================================================
*             CODE FOR OUTPUT USING 132 COLUMNS FORMAT
*=======================================================================
*
      ELSE
         IF( NDIGIT.LE.4 ) THEN
            DO 120 K1 = 1, N, 10
               K2 = MIN0( N, K1+9 )
  120       CONTINUE
*
         ELSE IF( NDIGIT.LE.6 ) THEN
            DO 140 K1 = 1, N, 8
               K2 = MIN0( N, K1+7 )
  140       CONTINUE
*
         ELSE IF( NDIGIT.LE.10 ) THEN
            DO 160 K1 = 1, N, 6
               K2 = MIN0( N, K1+5 )
  160       CONTINUE
*
         ELSE
            DO 180 K1 = 1, N, 5
               K2 = MIN0( N, K1+4 )
  180       CONTINUE
         END IF
      END IF
*
      RETURN
      END
