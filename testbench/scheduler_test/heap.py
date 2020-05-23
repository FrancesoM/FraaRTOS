#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import math
import random
import copy

class Node():
    def __init__(self,):
        pass
    
    
    
class Heap():
    def __init__(self,_array,_fcompare=lambda x,y:x>y):
        self.array = _array
        self.fcompare = _fcompare
        # print(self)
        # print("now heapify..")
        
        #build heap
        start = self.get_leaves_start_index()
        for i in range(start-1,-1,-1):
            self.max_heapify(i)
        
        # print(self)
        
        #self.check()
        
    def get(self,idx):
        return self.array[idx]

    def left_child(self,idx):
        ret_idx = 2*idx+1
        return ret_idx
 
        
    def right_child(self,idx):
        ret_idx = 2*idx+2
        return ret_idx

    
    def parent(self,idx):
        ret_idx = math.floor((idx-1)/2)
        return ret_idx

    def get_leaves_start_index(self):
        L = len(self.array)
        return int(L/2)
    
    def swap(self,idx1,idx2):
        # print("Swapping {}:{} and {}:{}".format(idx1,self.array[idx1],idx2,self.array[idx2]))
        temp = self.array[idx1]
        self.array[idx1] = self.array[idx2]
        self.array[idx2] = temp
        
    def max_heapify(self,idx):
        # get max child given index
        # if no children were bigger, this sub tree is already correct
        # else swap big <--> small and recursevely call on largest
        L = len(self.array)

        s = "Len:{} - IDX:{} - L:{} R:{}".format(L,idx,self.left_child(idx),self.right_child(idx))
        # print(s)
        
        l_idx = self.left_child(idx)
        # < L and is evaluated before so no exception not thrown if beyond max length
        largest_idx = idx
        if l_idx < L:
            if self.fcompare(self.get(l_idx) , self.get(idx)):
                largest_idx = l_idx
            

        # now compare with largest so we save one comparison - if largest was left and r is greater than largest
        # then it's also greater then idx 
        r_idx = self.right_child(idx)
        if r_idx < L:
            if self.fcompare(self.get(r_idx) , self.get(largest_idx)):
                largest_idx = r_idx
         
        try:
            s = "{} - L:{} R:{}".format(self.get(idx),self.get(l_idx),self.get(r_idx))
            # print(s)
        except IndexError:
            pass
            
        # need to swap if there is a larger in children  
        if largest_idx != idx:
            self.swap(idx,largest_idx)
            # now at largest_idx position there is the current
            self.max_heapify(largest_idx)
        
    def delete_root(self):
        self.swap(0,-1)
        ret = self.array.pop(-1)
        
        #rebuild heap
        start = self.get_leaves_start_index()
        for i in range(start-1,-1,-1):
            self.max_heapify(i)
            
        return ret
        
    def check(self):
        start = self.get_leaves_start_index()
        for i in range(start-1,-1,-1):
            try:
                # print ("{} > {}".format(self.get(i),self.get(self.left_child(i))))
                if( self.get(i) < self.get(self.left_child(i))):
                    print(self)
                    assert(False)
            except IndexError:
                print("No left child")    
            try:
                # print ("{} > {}".format(self.get(i),self.get(self.right_child(i))))
                if (self.get(i) < self.get(self.right_child(i))):
                    print(self)
                    assert(False)
            except IndexError:
                #print("No right child")
                pass
                

    
    def __str__(self):
        idx = 0
        level = 0
        
        ret_str = "{}:".format(idx)
        
        L = len(self.array)
        levels = int(math.floor(math.log(L,2)))
        max_elem = 1<<levels
        spacing_level_0 = int(max_elem)*2
        
        while( idx < L):
            elem_in_level = 1<<level
            for i in range(elem_in_level):
                if idx >= len(self.array):
                    break
                ret_str += "{} ({}:{}) ".format(" "*int((spacing_level_0/(level+1))),idx,str(self.array[idx]))
                idx+=1
            level += 1
            ret_str += "\n{}:".format(idx)
            
        return ret_str
            
        
def HeapSort(array,_fcompare=lambda x,y:x>y):
    print(array)
    H = Heap(array,_fcompare)
    array_sorted = []
    while(len(H.array) > 0):
        array_sorted.append(H.delete_root())
    
    print(array_sorted)

if __name__ == "__main__":
    
    test_number = 1
    
    #test heap class
    if test_number == 0:
        for i in range(1000):
            try:
                EL = random.randint(1,100)
                H = Heap([ random.randint(0,100) for i in range(EL)]  )
            except AssertionError:
                print( "Something wrong in the algorithm")
    
    elif test_number == 1:
    #test heap sort
        A = [ random.randint(0,100) for i in range(10)]
        HeapSort(copy.deepcopy(A), lambda x,y:x>y)
        HeapSort(copy.deepcopy(A), lambda x,y:x<y)
    
    
    
    
    