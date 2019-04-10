/***
 * filename: state_machine.c
 *
 * summary: This is the collection of a lot of the pods data and
 * where it will check for faults. The pod will always be running this code and
 * the state machine that lives here is inherently global as it applies to the
 * whole pod.
 *
 ***/

#include "badgerloop.h"
#include "data.h"
#include "state_machine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static void initState(state_t* state, char* name, stateTransition_t *(*action)(), int numTrans);
static void initTransition(stateTransition_t *transition, state_t *target, bool (*action)() );


extern data_t *data;

extern stateTransition_t * powerOnAction(void);
extern stateTransition_t * idleAction(void);
extern stateTransition_t * readyForPumpdownAction(void);
extern stateTransition_t * pumpdownAction(void);
extern stateTransition_t * readyForLaunchAction(void);
extern stateTransition_t * propulsionAction(void);
extern stateTransition_t * brakingAction(void);
extern stateTransition_t * stoppedAction(void);
extern stateTransition_t * crawlAction(void);
extern stateTransition_t * postRunAction(void);
extern stateTransition_t * safeToApproachAction(void);
extern stateTransition_t * secondaryBrakingAction(void);
extern stateTransition_t * preFaultAction(void);
extern stateTransition_t * runFaultAction(void);
extern stateTransition_t * postFaultAction(void);



stateMachine_t stateMachine;

/***
 * findState - Searches all the created states and returns the one with a matching name
 *
 * ARGS: char *stateName - Name of the state we are searching for. Check out state_machine.h
 * 	for options.
 *
 * RETURNS: state_t, the found state or NULL if that state doesnt exist
 */
state_t *findState(char *stateName) {
    for (int i = 0; i < NUM_STATES; i++) {
        if (strcmp(stateMachine.allStates[i]->name, stateName) == 0) {
            return stateMachine.allStates[i];
        }
    }
    return NULL;
}

/***
 * findTransition - Looks through a passed in states list of transitions
 * 	and identifies the one that leads to a specified target
 *
 * ARGS: state_t srcState - The state whose transitions we are searching through
 * 		 char *targName	  - The name of the state we want a transition to
 *
 * RETURNS: stateTransition_t the transition to the state we want, or NULL
 * 	if no such transition exists.
 */
stateTransition_t *findTransition(state_t *srcState, char *targName) {
    for (int i = 0; i < srcState->numTransitions; i++) {
        if (strcmp(srcState->transitions[i]->target->name, targName) == 0)
            return srcState->transitions[i];
    }
    return NULL;
}

/***
 * runStateMachine -
 *		Executes the current states action. A mini control loop
 * 	that will be called on every iteration of the overarching control loop. It
 *  will be able to then check if the current state should transition, and if so
 *  it should be able to execute that transition action as well and change the
 *  current state.
 *		It also is responsible for dealing with incoming state commands
 *  from the dashboard, after they have been parsed and appropriate flags have been
 *  set elsewhere.
 *
 */
void runStateMachine(void) {
    /* The cmd receiver will populate this field if we get an override */
    if (stateMachine.overrideStateName != NULL) {
        state_t *tempState = findState(stateMachine.overrideStateName);
        /* TODO We also need to execute a transition if it exists here */
        if (tempState != NULL)
            stateMachine.currState = tempState;
        stateMachine.overrideStateName = NULL;
    }

    /* execute the state and check if we should be transitioning */
	stateTransition_t *transition = stateMachine.currState->action();
    if (transition != NULL) {
		transition->action();
		stateMachine.currState = transition->target;
	}
}

/***
 * buildStateMachine - allocates memory for each state and creates the
 * 	state machine struct that will contain the meta data for the whole
 *  state machine. Its a globally available because it will be so commonly
 *  used.
 *
 */
void buildStateMachine(void) {
	    /* Create all of the states*/
	state_t *powerOff, *idle, *readyForPumpdown, *pumpdown,
			*readyForLaunch, *propulsion, *braking, *secondaryBraking,
			*stopped, *crawl, *rebrake, *postRun, *safeToApproach, *preFault,
			*runFault, *postFault;
	state_t **allStates = malloc(NUM_STATES * sizeof(state_t*));
    
	stateMachine.allStates = 
    {
            powerOff, idle, readyForPumpdown, pumpdown,
            readyForLaunch, propulsion, braking, secondaryBraking,
            stopped, crawl, postRun, safeToApproach, preFault, runFault, postFault
    };
    
    // This is the new API
    initPowerOff(powerOff);
    initIdle(idle);
    initReadyForPumpdown(readyForPumpdown);

    // TODO replace old way of creating states
	initState(pumpdown, PUMPDOWN_NAME, pumpdownAction, 1);
	initState(readyForLaunch, READY_NAME, readyForLaunchAction, 1);
	initState(propulsion, PROPULSION_NAME, propulsionAction, 1);
	initState(braking, BRAKING_NAME, brakingAction, 1);
	initState(stopped, STOPPED_NAME, stoppedAction, 1);
	initState(crawl, CRAWL_NAME, crawlAction, 1);
	initState(postRun, POST_RUN_NAME, postRunAction, 1);
	initState(safeToApproach, SAFE_TO_APPROACH_NAME, safeToApproachAction, 1);
	initState(preFault, PRE_RUN_FAULT_NAME, preFaultAction, 1);
	initState(runFault, RUN_FAULT_NAME, runFaultAction, 1);
	initState(postFault, POST_RUN_FAULT_NAME, postFaultAction, 1);
}

/***
 * initState - fills in the fields of the state_t struct.
 *
 * ARGS: state_t *state - allocated state (malloced most likely),
 *       stateTransition_t *(*action)() - a function pointer to a function that returns a state
 *			transition pointer. This is the meat of the state
 * 		 numTransitions - not implemented, fairly self explanatory
 */
static void initState(state_t* state, char* name, stateTransition_t *(*action)(), int numTransitions ) {
    static int indexInAllStates = 0;
    int i = 0;
    
    state = malloc(sizeof(state_t));
    if (state == NULL) {
        STATE_ERROR();
    }
    strncpy(state->name, name, strlen(name) );
    state->action = action;
    state->numTransitions = numTransitions;
    state->transitions = malloc(numTransitions * (sizeof(stateTransition_t *)));
    for (i = 0; i < numTransitions; i++) {
        state->transitions[i] = malloc(sizeof(stateTransition_t));
        state->transitions[i]->target = NULL;
    }
    stateMachine.allStates[indexInAllStates++] = state;
    
}

/***
 * initTransition - populates a transition struct
 *
 */
static void initTransition(stateTransition_t *transition, state_t *target, bool (*action)() ) {
    transition = malloc(sizeof(stateTransition_t));
    if (transition == NULL) return -1;
    transition->target;
}

static int addTransition(state_t *state, stateTransition_t *trans) {
    int i = 0;
    for (i = 0; state->transitions[i]->target != NULL || i < state->numTransitions; i++);
    if (i >= state->numTransitions) {
        fprintf(stderr, "[WARN] adding too many transitions! Ignoring extra.\n");
        return -1;
    }
    state->transitions[i] = trans;
}

static int initPowerOff(state_t *powerOff) {
    stateTransition_t *toIdle, *toPreFault;
	
    initState(powerOff, POWER_OFF_NAME, powerOnAction, 2);   
    
    initTransition(toIdle, findState(IDLE_NAME), NULL);
    initTransition(toPreFault, findState(PRE_RUN_FAULT_NAME), NULL); 
    addTransition(powerOff, toIdle);
    addTransition(powerOff, toPreFault);
    return 0;
}

static int initIdle(state_t *idle) {
    stateTransition_t *toReadyForPumpdown, *toPreFault;

    initState(idle, IDLE_NAME, idleAction, 2);

    initTransition(toReadyForPumpdown, findState(READY_FOR_PUMPDOWN_NAME), NULL);
    initTransition(toPreFault, findState(PRE_RUN_FAULT_NAME), NULL);
    addTransition(idle, toReadyForPumpdown);
    addTransition(idle, toPreFault);
    return 0;
}

static int initReadyForPumpdown(state_t *readyForPumpdown) {
    stateTransition_t *toPumpdown, *toPreFault;

    initState(readyForPumpdown, READY_FOR_PUMPDOWN_NAME, readyForPumpdownAction, 2);
    
    initTransition(toPumpdown, findState(PUMPDOWN_NAME), NULL);
    initTransition(toPreFault, findStaet(PRE_RUN_FAULT_NAME), NULL);
    addTransition(readyForPumpdown, toPumpdown);
    addTransition(readyForPumpdown, toPreFault);
    return 0;
}
