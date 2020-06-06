#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun  6 23:00:56 2020

@author: francescomaio
"""

import numpy as np



def DM_Scheduler(t,ready_queue):
    # DM scheduler policy which return the index of the task with the smallest deadline
    # note that this is not EDF, so priorities are fixed. 
    
    # Minimum element indices in list 
    # Using list comprehension + min() + enumerate() 
    temp  = min(ready_queue, key=lambda tau : tau.D)
    res = [i for i, j in enumerate(ready_queue) if j == temp][0]
    return res

def DM_Guarantee(task_list):
    
    #sort tasks by priority 
    task_list.sort( key=lambda tau: tau.D )
    
    """    foreach tau_i in TAU:
    I_i = sum j from 0 to i-1 ( C_j )
    do
        # If estimated I is this, calculate R at this point
        R_i = I_i + C_i
        if ( R_i > D_i ) return UNSCHEDULABLE
        
        #update interference based on last R guess
        I_i = sum_j_from_0_to_i-1 ( ceil( R_i / T_j ) * C_j  ) 
        
    while( I_i + C_i > R_i )
    """
    
    I = [0]*len(task_list)
    R = [0]*len(task_list)
    for i,tau in enumerate(task_list):
        
        higher_priority_tasks = task_list[:i]
        I[i] = sum( map( lambda t: t.C , higher_priority_tasks ) )
        while_condition = True
        
        while( while_condition ):
            R[i] = I[i] + tau.C
            if( R[i] > tau.D):
                return False
            I[i] = sum ( [np.ceil( R[i]/t.T) * t.C for t in higher_priority_tasks ] ) 
            
            while_condition = ( I[i] + tau.C > R[i] )
            
    return True
        
        
def EDF_Scheduler(t,ready_queue):
    
    # Earliest deadline first reorders the active queue based on the closest deadline
    # It could be done at runtime or offline depending on the tasks type. Since tasks are 
    # periodic in this OS Simulator class, the scheduling can be done offline. To be honest,
    # the simulator always performs a dynamic decision, but the guarantee can be explored completely offline
    
    #get time elapsed since release time (aka time passed since last period )
    time_elapsed_since_release = [ t%tau.T for tau in ready_queue  ]
    deadlines = [tau.D for tau in ready_queue ]
    
    time_to_deadline = [ d - t for d,t in zip(deadlines,time_elapsed_since_release) ]
    
    temp  = min(time_to_deadline)
    res = [i for i, j in enumerate(time_to_deadline) if j == temp][0]
    
    return res

def EDF_Guarantee(task_list):
    return True
    