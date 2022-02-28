/*
KTH Parallel programming ID1217, VT2022
Homework assignement 3

THE UNISEX BATHROOM PROBLEM
Suppose there is only one bathroom in your department. It can be used by any number of men or any number of women,
but not at the same time.
Develop and implement a multithreaded program that provides a fair solution to this problem using only semaphores for
synchronization. Represent men and women as threads that repeatedly work (sleep for a random amount of time) and use
the bathroom. Your program should allow any number of men or women to be in the bathroom at the same time. Your
solution should ensure the required exclusion and avoid deadlock, and ensure fairness, i.e. ensure that any person
(man or woman) which is waiting to enter the bathroom eventually gets to do so. Have the persons sleep for a random
amount of time between visits to the bathroom and have them sleep for a smaller random amount of time to simulate the
time it takes to be in the bathroom. Your program should print a trace of interesting simulation events.


*/

import java.util.concurrent.*;

class BinarySem extends Semaphore{
    private final Object lock = new Object(); // a dummy object,

    public BinarySem(boolean startReleased, boolean isFair){
        super((startReleased ? 1: 0), isFair);
    }
    @Override
    public void acquire() throws InterruptedException {
        super.acquire(1);
    }
    @Override
    public void release(){
        synchronized (lock){
            if(this.availablePermits() == 0){ super.release(); }
            else                                System.out.println("attempted releasing released permit");
        }
    }
}

class BathroomSurveillance{
    public int waitingCats, waitingDogs;       // number of threads in queue to use the bathroom
    public int pissingCats, pissingDogs;       // number of threads actively using the bathroom

    public BathroomSurveillance(){
        this.waitingCats = 0;
        this.waitingDogs = 0;
        this.pissingCats = 0;
        this.pissingDogs = 0;
    }
}

class Cat extends Thread {
    Semaphore semCat, semDog;
    BinarySem bsem, bathroomControl;
    String threadName;
    int bathroomVisits;
    BathroomSurveillance bs;

    public Cat(Semaphore semcat, Semaphore semdog, BinarySem bsem, BinarySem bathroomControl, String threadName, BathroomSurveillance bs){
        this.semCat = semcat;
        this.semDog = semdog;
        this.bsem = bsem;
        this.bathroomControl = bathroomControl;
        this.threadName = threadName;
        this.bs = bs;
        this.bathroomVisits = 0;
    }

    @Override
    public void run(){
        try{
            while(true){
                while(bs.pissingDogs < 1){
                    semCat.acquire();

                    bsem.acquire();
                    bs.pissingCats++;
                    if(bs.pissingCats == 1) {bathroomControl.acquire(); }
                    bsem.release();
                    // have a tinkle, then short sleep
                    Thread.sleep((int)(10000*Math.random()));
                    // leave bathroom, then long sleep
                    bsem.acquire();
                    bs.pissingCats--;
                    if(bs.pissingCats == 0){ bathroomControl.release(); }
                    bsem.release();
                    semCat.release();

                    Thread.sleep((int)(100000*Math.random()));
                }
            }
        }
        catch(InterruptedException ie){ System.out.println(ie);}
    }
}

class Dog extends Thread
{
    Semaphore semCat, semDog;
    BinarySem bsem, bathroomControl;
    String threadName;
    int bathroomVisits;
    BathroomSurveillance bs;

    public Dog(Semaphore semcat, Semaphore semdog, BinarySem bsem, BinarySem bathroomControl, String threadName, BathroomSurveillance bs){
        this.semCat = semcat;
        this.semDog = semdog;
        this.bsem = bsem;
        this.bathroomControl = bathroomControl;
        this.threadName = threadName;
        this.bs = bs;
        this.bathroomVisits = 0;
    }

    @Override
    public void run(){
        try{
            while(true){
                while(bs.pissingCats < 1){
                    semDog.acquire();

                    bsem.acquire();
                    bs.pissingDogs++;
                    if(bs.pissingDogs == 1) {bathroomControl.acquire();}
                    // have a tinkle, then short sleep
                    Thread.sleep((int)(10000*Math.random()));
                    // leave bathroom, then long sleep
                    bsem.acquire();
                    bs.pissingDogs--;
                    if(bs.pissingDogs == 0){ bathroomControl.release();}
                    bsem.release();
                    semDog.release();

                    Thread.sleep((int)(100000*Math.random()));
                }
            }
        }
        catch(InterruptedException ie){

        }
    }
}

public class Bathroom{
    public static void main(String args[]){
        int nCats, nDogs;
        boolean isFair;

        if(args.length < 3){
            nCats = 10;
            nDogs = 10;
            isFair = true;
        }
        else{
            nCats = Integer.parseInt(args[0]);
            nDogs = Integer.parseInt(args[1]);
            if (args[2].equals("0")) {
                isFair = false;
            } else {
                isFair = true;
            }
        }

        Semaphore catSem = new Semaphore(nCats, isFair);
        Semaphore dogSem = new Semaphore(nDogs, isFair);
        BinarySem binSem = new BinarySem(true,true);
        BinarySem bathroomControl = new BinarySem(true, true);

        BathroomSurveillance bs = new BathroomSurveillance();

        //Semaphore semcat, Semaphore semdog, BinarySem bsem, BinarySem bathroomControl, String threadName, BathroomSurveillance bs
        Cat[] cats = new Cat[nCats];
        Dog[] dogs = new Dog[nDogs];
        for (int i = 0; i < nCats; i++) {
            cats[i] = new Cat(catSem, dogSem, binSem, bathroomControl, String.format("bee %d", i + 1), bs);
            dogs[i] = new Dog(catSem, dogSem, binSem, bathroomControl, String.format("bee %d", i + 1), bs);
        }
        for (int i = 0; i < nCats; i++) {
            cats[i].start();
            dogs[i].start();
        }
    }
}