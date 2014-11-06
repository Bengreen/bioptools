/*************************************************************************

   Program:    rmspdb
   File:       rmspdb.c
   
   Version:    V1.0
   Date:       01.11.94
   Function:   Calculate RMS between 2 PDB files. Does no fitting.
   
   Copyright:  (c) Dr. Andrew C. R. Martin 1994
   Author:     Dr. Andrew C. R. Martin
   Address:    Biomolecular Structure & Modelling Unit,
               Department of Biochemistry & Molecular Biology,
               University College,
               Gower Street,
               London.
               WC1E 6BT.
   Phone:      (Home) +44 (0372) 275775
   EMail:      INTERNET: amartin@scitec.adsp.sub.org
                         martin@bsm.bioc.ucl.ac.uk
               UUCP:     ...{uunet|rutgers}!cbmehq!cbmuk!scitec!amartin
               JANET:    martin@uk.ac.ucl.bioc.bsm
               
**************************************************************************

   This program is not in the public domain, but it may be copied
   according to the conditions laid out in the accompanying file
   COPYING.DOC

   The code may be modified as required, but any modifications must be
   documented so that the person responsible can be identified. If someone
   else breaks this code, I don't want to be blamed for code that does not
   work! 

   The code may not be sold commercially or included as part of a 
   commercial product except as described in the file COPYING.DOC.

**************************************************************************

   Description:
   ============

**************************************************************************

   Usage:
   ======

**************************************************************************

   Revision History:
   =================

*************************************************************************/
/* Includes
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bioplib/pdb.h"
#include "bioplib/macros.h"

/************************************************************************/
/* Defines and macros
*/
#define MAXBUFF     160

#define ATOMS_NOH     0
#define ATOMS_ALL     1
#define ATOMS_NCAC    2
#define ATOMS_NCACO   3
#define ATOMS_CA      4

/************************************************************************/
/* Globals
*/

/************************************************************************/
/* Prototypes
*/
int main(int argc, char **argv);
BOOL SelectAndFixAtoms(PDB **pdb1, PDB **pdb2, int atoms);
void Usage(void);
BOOL ParseCmdLine(int argc, char **argv, char *file1, char *file2, 
                  int *atoms);

/************************************************************************/
/*>int main(int argc, char **argv)
   -------------------------------
   Main program for calculating RMS deviation with no fitting.

   01.11.94 Original    By: ACRM
*/
int main(int argc, char **argv)
{
   FILE *fp1,
        *fp2;
   char file1[MAXBUFF],
        file2[MAXBUFF];
   int  natoms,
        atoms = ATOMS_NOH;
   REAL rms;
   PDB  *pdb1,
        *pdb2;

   if(ParseCmdLine(argc, argv, file1, file2, &atoms))
   {
      /* Open the two PDB files                                         */
      if((fp1=fopen(file1,"r"))==NULL)
      {
         fprintf(stderr,"Unable to open file: %s\n",file1);
         return(1);
      }
      if((fp2=fopen(file2,"r"))==NULL)
      {
         fprintf(stderr,"Unable to open file: %s\n",file2);
         return(1);
      }
      
      /* Read the two PDB files                                         */
      if((pdb1 = ReadPDB(fp1,&natoms))==NULL)
      {
         fprintf(stderr,"No atoms read from file: %s\n",file1);
         return(1);
      }
      if((pdb2 = ReadPDB(fp2,&natoms))==NULL)
      {
         fprintf(stderr,"No atoms read from file: %s\n",file2);
         return(1);
      }

      /* Apply atom selection and fix atom order                        */
      if(SelectAndFixAtoms(&pdb1, &pdb2, atoms))
      {
         /* Calculate RMS between structures                            */
         rms = CalcRMSPDB(pdb1, pdb2);
         
         /* Print the result                                            */
         printf("RMS deviation over %s: %f\n",
                ((atoms == ATOMS_NOH) ? "heavy atoms" :
                 ((atoms == ATOMS_CA) ? "CA atoms" :
                  ((atoms == ATOMS_NCAC) ? "N, CA, C atoms"   :
                   ((atoms == ATOMS_NCACO) ? "N, CA, C, O atoms" :
                    "all atoms")))),
                rms);
      }
      else
      {
         return(1);
      }
   }
   else
   {
      Usage();
   }

   return(0);
}


/************************************************************************/
/*>BOOL SelectAndFixAtoms(PDB **pdb1, PDB **pdb2, int atoms)
   ---------------------------------------------------------
   Apply atom selections and fix atom order ready for RMSd calculation.

   01.11.94 Original    By: ACRM
*/
BOOL SelectAndFixAtoms(PDB **pdb1, PDB **pdb2, int atoms)
{
   PDB  *pdbin1  = *pdb1,
        *pdbin2  = *pdb2,
        *pdbout1 = NULL,
        *pdbout2 = NULL;
   char *sel[4];
   int  natoms1 = 0,
        natoms2 = 0;

   SELECT(sel[0],"CA  ");
   SELECT(sel[1],"N   ");
   SELECT(sel[2],"C   ");
   SELECT(sel[3],"O   ");

   /* Apply appropriate atom selection                                  */
   switch(atoms)
   {
   case ATOMS_ALL:
      pdbout1 = pdbin1;
      pdbout2 = pdbin2;
      break;
   case ATOMS_NOH:
      pdbout1 = StripHPDB(pdbin1, &natoms1);
      pdbout2 = StripHPDB(pdbin2, &natoms2);
      FREELIST(pdbin1, PDB);
      FREELIST(pdbin2, PDB);
      break;
   case ATOMS_NCAC:
      pdbout1 = SelectAtomsPDB(pdbin1, 3, sel, &natoms1);
      pdbout2 = SelectAtomsPDB(pdbin2, 3, sel, &natoms2);
      FREELIST(pdbin1, PDB);
      FREELIST(pdbin2, PDB);
      break;
   case ATOMS_NCACO:
      pdbout1 = SelectAtomsPDB(pdbin1, 4, sel, &natoms1);
      pdbout2 = SelectAtomsPDB(pdbin2, 4, sel, &natoms2);
      FREELIST(pdbin1, PDB);
      FREELIST(pdbin2, PDB);
      break;
   case ATOMS_CA:
      pdbout1 = SelectAtomsPDB(pdbin1, 1, sel, &natoms1);
      pdbout2 = SelectAtomsPDB(pdbin2, 1, sel, &natoms2);
      FREELIST(pdbin1, PDB);
      FREELIST(pdbin2, PDB);
      break;
   default:
      fprintf(stderr,"Internal illegal option in SelectAndFixAtoms()\n");
      break;
   }

   /* Check memory allocation succeeded                                 */
   if(pdbout1 == NULL || pdbout2 == NULL)
   {
      fprintf(stderr,"Unable to allocate memory for atom selection\n");
      return(FALSE);
   }
   
   /* Check number of atoms matches                                     */
   if(natoms1 != natoms2)
   {
      fprintf(stderr,"Number of atoms does not match\n");
      return(FALSE);
   }
   
   if(atoms != ATOMS_ALL)
   {
      /* Correct the atom order. The flags are for padding with missing 
         atoms and renumbering - we don't want to do either of these!
      */
  
      if((*pdb1 = FixOrderPDB(pdbout1, FALSE, FALSE))==NULL)
      {
         fprintf(stderr,"Unable to fix atom order\n");
         return(FALSE);
      }
      
      if((*pdb2 = FixOrderPDB(pdbout2, FALSE, FALSE))==NULL)
      {
         fprintf(stderr,"Unable to fix atom order\n");
         return(FALSE);
      }
   }
   
   return(TRUE);
}


/************************************************************************/
/*>BOOL ParseCmdLine(int argc, char **argv, char *file1, char *file2, 
                     int *atoms)
   ---------------------------------------------------------------------
   Input:   int    argc         Argument count
            char   **argv       Argument array
   Output:  char   *file1       Input file (or blank string)
            char   *file2       Output file (or blank string)
            int    *atoms       Atom selection (ATOMS_*)
   Returns: BOOL                Success?

   Parse the command line
   
   05.07.94 Original    By: ACRM
*/
BOOL ParseCmdLine(int argc, char **argv, char *file1, char *file2, 
                  int *atoms)
{
   argc--;
   argv++;

   file1[0] = file2[0] = '\0';
   *atoms = ATOMS_NOH;
   
   while(argc)
   {
      if(argv[0][0] == '-')
      {
         switch(argv[0][1])
         {
         case 'h':
            *atoms = ATOMS_ALL;
            break;
         case 'm':
            *atoms = ATOMS_NCACO;
            break;
         case 'b':
            *atoms = ATOMS_NCAC;
            break;
         case 'c':
            *atoms = ATOMS_CA;
            break;
         default:
            return(FALSE);
            break;
         }
      }
      else
      {
         /* Check that there are exactly 2 arguments left               */
         if(argc != 2)
            return(FALSE);
         
         /* Copy the arguments                                          */
         strcpy(file1, argv[0]);
         strcpy(file2, argv[1]);
            
         return(TRUE);
      }
      argc--;
      argv++;
   }
   
   return(FALSE);
}

/************************************************************************/
/*>void Usage(void)
   ----------------
   Prints a usage message

   01.11.94 Original    By: ACRM
*/
void Usage(void)
{
   fprintf(stderr,"\nRmsPDB V1.0 (c) 1994, Andrew C.R. Martin, UCL\n");
   fprintf(stderr,"Usage: rmspdb [-h] [-c] [-b] [-m] <in1.pdb> \
<in2.pdb>\n");
   fprintf(stderr,"                -h Include hydrogens\n");
   fprintf(stderr,"                -c CAs only\n");
   fprintf(stderr,"                -b N, CA, C only\n");
   fprintf(stderr,"                -m N, CA, C, O only\n\n");
   fprintf(stderr,"Calculates an RMS between 2 PDB files. No fitting is \
performed.\n");
   fprintf(stderr,"N.B. With the -h option, the atom order must match in \
the two files before fitting\n\n");
}

