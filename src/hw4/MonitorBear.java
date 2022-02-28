/*
ID1217 Parallel programming, KTH, VT2022
Astrid Lindh

Problem 6 - recreate the Bear and the Honeybees problem without the use of semaphores

"Develop a monitor (with condition variables) to synchronize the actions of the bear and honeybees, i.e. develop a
monitor that represents the pot of honey. Define the monitor's operations and their implementation. Implement a
multithreaded application in Java or C++ to simulate the actions of the bear and honeybees represented as concurrent
threads and the pot represented as as the monitor. Is your solution fair? Explain in comments in the source code."


fairness: unless a producer thread is forced to sleep after aqcuiring access to the Monitor's deposit method, it seems
to have a much higher likelihood of accessing the Monitor object again ahead of other producers (once a thread gets access it might continually get acess again and again about 100-200 times, sometimes shorter. Eventually, another
producer may get access.
This Monitor implementation is unfair in itself, but by forcing every producer thread to sleep for some time after
having succesfully acquired the Monitor object
 */

/*
Given are n honeybees and a hungry bear. They share a pot of honey.
The pot is initially empty; its capacity is H portions of honey. The bear sleeps until the pot is full, then eats all
the honey and goes back to sleep. Each bee repeatedly gathers one portion of honey and puts it in the pot;
the bee who fills the pot awakens the bear.

Develop and implement a multithreaded program to simulate the actions of the bear and honeybees.
Represent the bear and honeybees as concurrent threads (i.e. a "bear" thread and an array of "honeybee" threads),
and the pot as a critical shared resource that can be accesses by at most one thread at a time.
Use only semaphores for synchronization. Your program should print a trace of interesting simulation events.
Is your solution fair (w.r.t. honeybees)? Explain when presenting homework.
 */

import java.util.concurrent.*;
import java.lang.Thread;

// Monitor class
class HoneyPot{
    int capacity;           // size of buffer
    int currentContent;     // current amount of objects in the buffer

    public HoneyPot(int cap){
        this.capacity = cap;
        this.currentContent = 0;
    }

    // many consumers make deposits
    public synchronized void deposit(String threadName, int timesActivated){
        try{
            // if pot is full, wait for bear to empty it
            while(currentContent >= capacity){
                wait();
            }
            // otherwise, deposit a drop
            currentContent++;
            System.out.println(String.format("%s goes to work, activated %d times (pot contains %d units)", threadName, timesActivated + 1, currentContent));
            notifyAll();
        }
        catch(Exception e){System.out.println(e);}
    }

    // single consumer consumes
    public synchronized void consume(int potsEmptied){
        try{
            while(currentContent < capacity){
                wait();
            }
            currentContent = 0;
            System.out.println(String.format("BEAR WAKES UP!    total pots of honey eaten: %d", potsEmptied + 1));
            notifyAll();
        }
        catch(InterruptedException ie){System.out.println(ie);}
    }
}

// consumer class - single consumer thread should run parallel with producer threads
class ConsumerBear extends Thread{
    int potsEmptied;
    HoneyPot pot;
    String threadName;

    public ConsumerBear(HoneyPot pot, String threadName){
        this.pot = pot;
        this.threadName = threadName;
        potsEmptied = 0;
    }

    @Override
    public void run(){
        while(true){
            try{
                System.out.println(String.format("%s is awoken", threadName));
                pot.consume(potsEmptied);
                potsEmptied++;
                // consumer does not need to sleep
                // Thread.sleep((int)(10000*Math.random()));
            }
            catch(Exception e){ System.out.println(e); }
        }
    }
}

// producer class - multiple producer threads should run parallel with each other and consumer
class ProducerBee extends Thread{
    int timesActivated;
    HoneyPot pot;
    String threadName;

    public ProducerBee(HoneyPot pot, String threadName){
        this.pot = pot;
        this.threadName = threadName;
        this.timesActivated = 0;        // increments every monitor method is accessed
    }

    @Override
    public void run(){
        while(true){
            try{
                pot.deposit(threadName, timesActivated);
                timesActivated++;
                // WITHOUT SLEEP, EXTREMELY UNFAIR
                Thread.sleep((int)(3000*Math.random()));
            }
            catch (Exception e){
                System.out.println(e);
            }
        }
    }
}


// driver class
public class MonitorBear{
    public static void main(String args[]) {
        int nBees, potCapacity;
        boolean isFair;

        try {
            if (args.length < 2) {
                nBees = 5;
                potCapacity = 12;
                System.out.println(String.format("Configured to %d bees, honey pot contains %d drops of honey", nBees, potCapacity));
            } else {
                nBees = Integer.parseInt(args[0]);
                potCapacity = Integer.parseInt(args[1]);
                System.out.println(String.format("Configured to %d bees, honey pot contains %d drops of honey", nBees, potCapacity));
            }

            HoneyPot pot = new HoneyPot(potCapacity);
            ConsumerBear bear = new ConsumerBear(pot, "Bear");
            bear.start();
            ProducerBee[] bees = new ProducerBee[nBees];
            for (int i = 0; i < nBees; i++) {
                bees[i] = new ProducerBee(pot, String.format("bee %d", i + 1));
            }
            for (int i = 0; i < nBees; i++) {
                bees[i].start();
            }
        }
        catch (Exception e) {
            System.out.println(e);
        }
    }
}