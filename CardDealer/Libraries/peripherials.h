/*
 * peripherials.c
 *
 *  Created on: 24 gen 2023
 *      Author: Francesco Olivieri
 */

#ifndef LIBRARIES_PERIPHERIALS_C_
#define LIBRARIES_PERIPHERIALS_C_

#include <stdio.h>
#include <ti/grlib/grlib.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* ---------- Step Motor ---------- */

/*
 * It moves the step motor by 0.17578° in the
 * direction of
 * @param move_forward if true moves counter clockwise
 *              otherwise moves clockwise
 */
void SM_makeStep(bool move_forward);

/* ---------- Distance Sensor ---------- */

/*
 * Sends a trigger to the distance sensor in order to
 * perform a distance measurement
 */
void DS_sendTrigger();

/* ---------- DC Motor ---------- */

/*
 * Starts the DC Motor in a counter clockwise direction
 */
void DCM_moveForward();

/*
 * Starts the DC Motor in a clockwise direction
 */
void DCM_moveBackward();

/*
 * Turn off the DC Motor
 */
void DCM_turnOff();



#endif /* LIBRARIES_PERIPHERIALS_C_ */
