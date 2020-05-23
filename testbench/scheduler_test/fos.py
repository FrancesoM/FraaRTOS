#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Mar  6 00:10:26 2020

@author: francescomaio
"""

STATE_RUN = "RUN"
STATE_WAIT = "WAIT"


# ======================== TASK CLASS ========================= #

class Task(object):
    def __init__(self,ID,prio,deadline,runtime):
        assert(deadline >= runtime)
        self.ID = ID
        self.prio = prio
        self.dead = deadline
        self.runtime = runtime
        self.time_has_run = 0
        #all start as being STATE_WAIT and they will arrive
        self.STATE = STATE_WAIT
        
    def request(self,time):
        self.arrive_time = time 
        self.time_has_run = 0
        self.STATE = STATE_RUN
        
    def run(self):
        self.time_has_run += 1
        print(self)
        if( self.time_has_run >= self.runtime ):
            print ( self.ID, "had been served <-----------------" )
            self.STATE = STATE_WAIT
        
    def __str__(self):
        perc = self.time_has_run/self.runtime if self.arrive_time != 0 else 0
        fill = int(perc*15)
        nofill = 15-fill
        return  """ 
===== ID: {}  ======
= prio {}, dead {} =
= target   = {}    =
= time run = {}    =
= arr time = {}    =
= [{}] =
                """.format( self.ID,
                            self.prio,
                            self.dead,
                            self.runtime,
                            self.time_has_run,
                            self.arrive_time,
                            ("*"*fill)+(" "*nofill))

# ======================== OS IMPL CLASS ========================= #

# Implementaion with arrays 
class OS_Array(object):
    def __init__(self,tasks):
        self.tasks = tasks
        self.tasks.append(Task("IDLE",10,100,100))
        self.tasks[-1].request(0)
        #this to say start always from IDLE
        self.idle_id = len(self.tasks)-1
        self.curridx = self.idle_id
        self.idx_of_tasks_at_run_state = [0]*len(self.tasks)
        #the idle is always 
        self.idx_of_tasks_at_run_state[0] = self.idle_id
        self.ptr_idx_of_tasks_at_run_state = 0
        
    def check_if_missed(self,time):
        for i in range(len(self.tasks)):
            if self.tasks[i].STATE == STATE_RUN:
                if time - self.tasks[i].arrive_time > self.tasks[i].dead:
                    raise Exception("Task {} has missed deadline".format(self.tasks[i].ID))
        
                    
    def schedule(self):
        
        print("==== run list: {} ====".format(self.idx_of_tasks_at_run_state))
    
        # travel the entire array to create the run queue - only those tasks which are waiting to be scheduled 
        # O(n)
        self.ptr_idx_of_tasks_at_run_state = 0
        for i in range(len(self.tasks)):
            if self.tasks[i].STATE == STATE_RUN:
                self.idx_of_tasks_at_run_state[self.ptr_idx_of_tasks_at_run_state] = i
                self.ptr_idx_of_tasks_at_run_state+=1
        
        
        
        # assume prio0 finishes and there are no others to be scheduled
        # in theory the scheduler must let IDLE run but it cannot do it because
        # "current" priority is too low  
        
        
        #if the current is not completed it can be pre-empted 
        if self.tasks[self.curridx].STATE == STATE_RUN:
            min_prio = self.tasks[self.curridx].prio
            min_dead = self.tasks[self.curridx].dead
            schedule_this = self.curridx
        #else take them from IDLE
        #the effect is that if idle is the only one in the run queue it will keep running
        #else it will preempted by any possible task
        else:
            min_prio = self.tasks[-1].prio
            min_dead = self.tasks[-1].dead
            schedule_this = self.idle_id
        

        # I am tempted to say this is at the worst case (all tasks must run ) an O(n)    
        for j in range(self.ptr_idx_of_tasks_at_run_state):
            
            tnext = self.idx_of_tasks_at_run_state[j]
            
            tprio = self.tasks[tnext].prio
            tdead = self.tasks[tnext].dead
            
            
            if tprio == min_prio:
                #if the next deadline is shorter schedule this 
                if tdead < min_dead:
                    schedule_this = tnext
            
            #there is a task with higher priority    
            if tprio < min_prio:
                #update the min prio
                min_prio = tprio
                min_dead = tdead
                schedule_this = tnext
            
        print("Schedule from {} to {}".format( self.tasks[self.curridx].ID, self.tasks[schedule_this].ID )      )      
        
        # in the OS the next step is to swap execution and update the current with the "next"
        # here it's just a matter of updating curridx
        
        self.curridx = schedule_this
        
        
    def getTaskCanRequest(self):
        tasks_that_can_request = []
        for t in self.tasks:
            if t.STATE == STATE_WAIT:
                tasks_that_can_request.append(t.ID)
        return tasks_that_can_request
    
    def setTaskRequest(self,ID,time):
        for i in range(len(self.tasks)):
            if self.tasks[i].ID == ID:
                self.tasks[i].request(time)
    
    def run_step(self):
        self.tasks[self.curridx].run()
        
        
        
        
        
        
        
        
        
        
        
        