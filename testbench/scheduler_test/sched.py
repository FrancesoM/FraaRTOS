#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Mar  5 00:21:42 2020

@author: francescomaio
"""
from __future__ import division
import random as rn
import fos
#TODO: transform all prints in logging 
#import logging


# ======================== SIMULATION CLASS ========================= #

class Sim(object):
    def __init__(self,os):
        
        self.time = 0
        self.os = os
        
        
    def sim_request(self):

        #add one random task simulating the request
        #but add only if task have been STATE - otherwise it cannot request again 
        
        #in order to do this creates an array of the possible threads that can request now
        #they cannot request if
        # - they are already running
        # - the deadline has not passed since they are arrived - how to impl?
        tasks_that_can_request = self.os.getTaskCanRequest()
        
        #now pick one randomly - if there is one
        if len(tasks_that_can_request) > 0:
            #get index
            random_idx = rn.randint(0,len(tasks_that_can_request)-1)
            ID = tasks_that_can_request[random_idx]
            #use index 
            self.os.setTaskRequest(ID,self.time)
            print( "Request from thread {}".format( ID ) ) 
        else:
            print("No tasks can request -- all waiting to finish")
 
    
          
        
    def advance(self):
        
        print ("====== t: {} ======== ".format(self.time))
        
        # hard fault if missed deadline 
        self.os.check_if_missed(self.time)
        
        # simulate interrupt request that "wakes up" a task which was at WAIT state
        self.sim_request()
        
        # call the schedule internally on the os - this can change but the simulation doesn't care
        self.os.schedule()
        
        # run the current thrad aka advance in time 
        self.os.run_step()
        
        # advance time step by one
        self.time+=1
        
        
    def simulate(self,T):
        for t in range(T):
            self.advance()

        

        
    
    
if __name__ == "__main__":

    tasks = [    fos.Task("0 HIGH",1,15,4),
                 fos.Task("1 MEDIUM",2,20,10),
                 fos.Task("2 CRITICAL",0,12,4),
                 fos.Task("3 MEDIUM",2,11,3),
                 fos.Task("4 CRITICAL",0,13,2)]
    
    myOS = fos.OS_Array(tasks)
    
    mySimulation = Sim(myOS)
    mySimulation.simulate(20)



    

#print("schedule this: \t'{}'\nfrom this \t'{}'".format(tasks[schedule_this],tasks[curr]))