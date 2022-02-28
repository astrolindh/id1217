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
import java.util.concurrent.Semaphore;
import java.lang.Thread;

class HoneyPot{
    int capacity;           // size of buffer
    int currentContent;     // amount of objects in the buffer

    public HoneyPot(int cap){
        this.capacity = cap;
        this.currentContent = 0;
    }

    public boolean isFull(){
        if(currentContent >= capacity) { return true;}
        return false;
    }
    public int getCurrentContent(){ return currentContent; }
    public void incrementCurrentContent(){
        currentContent++;
        // System.out.println(String.format("%d drops of honey in the pot", currentContent));
    }
    public void emptyPot(){ currentContent = 0;}
}

// single consumer class
class ConsumerBear extends Thread{
    Semaphore prod, con;
    int potsEmptied;
    HoneyPot pot;
    String threadName;

    public ConsumerBear(Semaphore prod, Semaphore con, HoneyPot pot, String threadName){
        this.prod = prod;
        this.con = con;
        this.pot = pot;
        this.threadName = threadName;
        potsEmptied = 0;
    }

    @Override
    public void run(){
        try{
            while(true){
                con.acquire();
                if(pot.isFull()){
                    System.out.println(String.format("%s wakes up and eats the honey", threadName));
                    pot.emptyPot();
                    potsEmptied++;
                    System.out.println(String.format("%s goes back to sleep (%d pots of honey eaten)", threadName, potsEmptied));

                }
                else{
                    System.out.println(String.format("%s says NO HONEY IN THE POT?!?!?!", threadName));
                }
                prod.release();
            }
        }
        catch (InterruptedException ie){
            System.out.println(ie);
        }
    }
}

// multiple producer
class ProducerBee extends Thread{
    Semaphore prod, con;
    int timesActivated;
    HoneyPot pot;
    String threadName;

    public ProducerBee(Semaphore prod, Semaphore con, HoneyPot pot, String threadName){
        this.prod = prod;
        this.con = con;
        this.pot = pot;
        this.threadName = threadName;
        this.timesActivated = 0;        // increments every time permit is acquired, to measure fairness
    }

    @Override
    public void run(){
        try{
            while(true){
                prod.acquire();
                timesActivated++;
                System.out.println(String.format("%s goes to work, activated %d times ", threadName, timesActivated));
                // deliver honey if pot is fukk
                if(!pot.isFull()){
                    pot.incrementCurrentContent();

                }
                else{
                    System.out.println(String.format("%s says BZZT BZZT %d drops of honey in the pot!", threadName, pot.getCurrentContent()));
                    con.release();
                }
                // let another bee inspect the honeypot
                prod.release();
                // bee takes a break
                Thread.sleep((int)(10000*Math.random()));
            }
        }
        catch (InterruptedException ie){
            System.out.println(ie);
        }
    }
}




// driver class
public class HungryBear{
    public static void main(String args[]) {
        // TODO fill out
        int nBees, potCapacity;
        boolean isFair;

        try {
            if (args.length < 3) {
                nBees = 5;
                potCapacity = 12;
                isFair = true;
                System.out.println(String.format("Configured to %d bees, honey pot contains %d drops of honey, and fairness is: %b", nBees, potCapacity, isFair));
            } else {
                nBees = Integer.parseInt(args[0]);
                potCapacity = Integer.parseInt(args[1]);
                if (args[2].equals("0")) {
                    isFair = false;
                } else {
                    isFair = true;
                }

                System.out.println(String.format("Configured to %d bees, honey pot contains %d drops of honey, and fairness is: %b", nBees, potCapacity, isFair));
            }

            // initialise shared variables
            Semaphore consumer = new Semaphore(1, isFair);
            Semaphore producer = new Semaphore(1, isFair);
            HoneyPot pot = new HoneyPot(potCapacity);

            ConsumerBear bear = new ConsumerBear(producer, consumer, pot, "Bear");
            bear.start();

            ProducerBee[] bees = new ProducerBee[nBees];
            for (int i = 0; i < nBees; i++) {
                bees[i] = new ProducerBee(producer, consumer, pot, String.format("bee %d", i + 1));
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