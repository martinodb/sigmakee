/*-----------------------------------------------------------------------

File  : che_heuristics.c

Author: Stephan Schulz

Contents
 
  High-Level interface functions to the heuristics module.


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 02:14:51 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_heuristics.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


HeuristicAssocCell HeuristicsTable[]=
{
   {HEU_AUTO_MODE,      "Auto",       HCBAutoModeCreate},
   {HEU_AUTO_MODE_CASC, "AutoCASC",   HCBCASCAutoModeCreate},
   {HEU_AUTO_MODE_DEV,  "AutoDev",    HCBDevAutoModeCreate},
   {HEU_AUTO_SCHED0,    "AutoSched0", HCBAutoSched0Create},
   {HEU_AUTO_SCHED1,    "AutoSched1", HCBAutoSched1Create},
   {HEU_AUTO_SCHED2,    "AutoSched2", HCBAutoSched2Create},
   {HEU_AUTO_SCHED3,    "AutoSched3", HCBAutoSched3Create},
   {HEU_AUTO_SCHED4,    "AutoSched4", HCBAutoSched4Create},
   {HEU_NO_HEURISTIC, NULL,           (HCBCreateFun)NULL}
};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: finalize_auto_parms()
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void finalize_auto_parms(char* modename, char* hname,
                         ProofControl_p control,
                         HeuristicParms_p parms, 
                         SpecFeature_p spec)
{


   if(OutputLevel+1)
   {
      fprintf(GlobalOut, 
	      "# %s selected heuristic %s\n"
	      "# and selection function %s.\n#\n", 
              modename, 
              hname, 
	      GetLitSelName(control->heuristic_parms.selection_strategy)); 
   }
   if(parms->mem_limit>2 && (parms->delete_bad_limit ==
			     DEFAULT_DELETE_BAD_LIMIT))
   {
      control->heuristic_parms.delete_bad_limit =
	 (float)(parms->mem_limit-2)*0.7;
   }
   if(SpecNoEq(spec))
   {
      control->heuristic_parms.ac_handling = NoACHandling;
      OUTPRINT(1, "# No equality, disabling AC handling.\n#\n");
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: HCBCreate() 
//
//   Given a name and the necessary parameters, return a HCB. HCB's
//   must be registered in control->hcbs!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

HCB_p HCBCreate(char* name, HCBARGUMENTS)
{
   int    i;
   DStr_p err;

   assert(control->ocb);

   for(i=0; HeuristicsTable[i].heuristic; i++)
   {
      if(strcmp(HeuristicsTable[i].name, name)==0)
      {
	 return HeuristicsTable[i].heuristic_create(state,control,parms);
      }
   }
   err = DStrAlloc();
   DStrAppendStr(err, "Heuristic \"");
   DStrAppendStr(err, name);
   DStrAppendStr(err, "\" unknown");
   Error(DStrView(err), USAGE_ERROR);
   DStrFree(err);
   
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: GetHeuristic()
//
//   Given a string (either a name or a Heuristic-Definition), return
//   a corresponding HCB.
//
// Global Variables: -
//
// Side Effects    : May create HCB's and WFCB's
//
/----------------------------------------------------------------------*/

HCB_p GetHeuristic(char* source, HCBARGUMENTS)
{
   char*     name;
   Scanner_p in;
   HCB_p     res;

   /* StreamTypeOptionString is a hack here...it will give wrong error
      messages for internal strings (but the function will not be
      called with internal strings anyways...) */
   in=CreateScanner(StreamTypeOptionString, source, true, NULL);
   if(TestInpTok(in, OpenBracket))
   {
      HeuristicDefParse(control->hcbs, in, control->wfcbs,
			control->ocb, state);
      name = SecureStrdup("Default");
   }
   else
   {      
      name = SecureStrdup(DStrView(AktToken(in)->literal));
      AcceptInpTok(in, Identifier);
   }
   DestroyScanner(in);
   res = HCBAdminFindHCB(control->hcbs, name);
   if(!res)
   {
      res = HCBCreate(name, state, control, parms);
      assert(res);
   }
   FREE(name);
   return res;
}



#define CHE_HEURISTICS_INTERNAL

/*-----------------------------------------------------------------------
//
// Function: HCBAutoModeCreate()
//
//   Analyse the proof problem and return an hopefully suitable
//   heuristic. 
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO
HCB_p HCBAutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");   
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("Auto-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO


/*-----------------------------------------------------------------------
//
// Function: HCBCASCAutoModeCreate()
//
//   Analyse the proof problem and return an hopefully suitable
//   heuristic. This is the CASC-20 auto mode
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO_CASC
HCB_p HCBCASCAutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits =  SpecLimitsAlloc();

   limits->ax_some_limit        = 46;
   limits->ax_many_limit        = 205;
   limits->lit_some_limit       = 212;
   limits->lit_many_limit       = 620;
   limits->term_medium_limit    = 163;
   limits->term_large_limit     = 2270;
   limits->far_sum_medium_limit = 5;
   limits->far_sum_large_limit  = 24;
   limits->depth_medium_limit   = 0;
   limits->depth_deep_limit     = 6;
   limits->gpc_absolute         = true;
   limits->gpc_few_limit        = 1;
   limits->gpc_many_limit       = 3;
   limits->ngu_absolute         = true;
   limits->ngu_few_limit        = 1;
   limits->ngu_many_limit       = 3;

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");   
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("Auto-Mode (CASC-20)", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_CASC


/*-----------------------------------------------------------------------
//
// Function: HCBDevAutoModeCreate()
//
//   Analyse the proof problem and return an hopefully suitable
//   heuristic, using the latest development auto mode.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/


#define CHE_HEURISTICS_AUTO_DEV
HCB_p HCBDevAutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("AutoDev-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_DEV


/*-----------------------------------------------------------------------
//
// Function: HCBAutoSched0Create()
// Function: HCBAutoSched1Create()
// Function: HCBAutoSched2Create()
// Function: HCBAutoSched3Create()
// Function: HCBAutoSched4Create()
//
//   Auto-mode variants for strategy-scheduling
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO_SCHED0
HCB_p HCBAutoSched0Create(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("AutoSched0-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_SCHED0

#define CHE_HEURISTICS_AUTO_SCHED1
HCB_p HCBAutoSched1Create(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("AutoSched1-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_SCHED1

#define CHE_HEURISTICS_AUTO_SCHED2
HCB_p HCBAutoSched2Create(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("AutoSched2-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_SCHED2

#define CHE_HEURISTICS_AUTO_SCHED3
HCB_p HCBAutoSched3Create(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("AutoSched3-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_SCHED3


#define CHE_HEURISTICS_AUTO_SCHED4
HCB_p HCBAutoSched4Create(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits = CreateDefaultSpecLimits();

   control->heuristic_parms.selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
#include "che_auto_cases.c"
   SpecLimitsCellFree(limits);

   finalize_auto_parms("AutoSched4-Mode", res, control, parms, spec);

   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_SCHED4

#undef CHE_HEURISTICS_INTERNAL


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


