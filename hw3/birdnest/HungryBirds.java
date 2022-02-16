/*
Given are n baby birds and one parent bird. The baby birds eat out of a common dish that initially contains W worms.
Each baby bird repeatedly takes a worm, eats it, sleeps for a while, takes another worm, and so on.
If the dish is empty, the baby bird who discovers the empty dish chirps real loud to awaken the parent bird.
The parent bird flies off and gathers W more worms, puts them in the dish, and then waits for the dish to be
empty again. This pattern repeats forever.

Develop and implement a multithreaded program to simulate the actions of the birds.
Represent the birds as concurrent threads (i.e. array of "babyBird" threads and a "parentBird" thread),
and the dish as a critical shared resource that can be accessed by at most one bird at a time. Use only semaphores
for synchronization.
Your program should print a trace of interesting simulation events.
Is your solution fair? Explain when presenting homework.

DO NOT PROGRAM SYNCHRONISED BLOCKS OR METHODS, USE ONLY SEMAPHORES

This is an example of the producer-consumers problem, with one Producer and many Consumers
 */


import java.util.concurrent.*;
import java.lang.System.*;
import java.io.*;


// class Dish of Worms. Should be protected by a shared semaphore. Equivalent to the buffer in the producer-consumers problem
/*
At most one thread can access the Dish at the same time:
    - Chicks, to eat worms
    - Parent, to refill
 */
class Dish
{
    int worms = 0;

    public int getNumberWorms(){ return worms; }
}

// class producent Parent - the Producer
class ParentBird extends Thread {
    Semaphore prod, con;
    Dish dish;
    int nWorms;
    String threadName;

    public ParentBird(Semaphore prod, Semaphore con, Dish dish, int nWorms, String threadName) {
        super(threadName);
        this.prod = prod;
        this.con = con;
        this.dish = dish;
        this.nWorms = nWorms;
        this.threadName = threadName;
    }

    /* If the dish is empty, the baby bird who discovers the empty dish chirps real loud to awaken the parent bird.
    The parent bird flies off and gathers W more worms, puts them in the dish, and then waits for the dish to be
    empty again

    will only activate if producer semaphore is available (this signal given by hungty Chick)
    refills bowl
    sets producer semaphore to 0, consumer semaphore to 1
     */

    @Override
    public void run() {
        // TODO

        // sleep, waiting for signal to wake up (chirp)
        // refill Dish with more worms
        // go to "sleep"
        while(true){
            try{
                // P on producer semaphore - parent gets more worms
                System.out.println("parent waits for a chirp");
                prod.acquire();
                System.out.println("Parent bird has woken up!");
                dish.worms += nWorms;
                System.out.println(String.format("Parent bird refilled the dish with %d worms, now there's %d worms in the dish", nWorms, dish.getNumberWorms()));

                // V on consumer semaphore - allow chicks to eat
                con.release();
            }
            catch (InterruptedException ie){
                System.out.println("Interruption in ParentBird");
                System.out.println(ie);
            }
        }
    }
}

// class consumer Chick - the Consumer
class Chick extends Thread {
    Semaphore prod, con;
    Dish dish;
    String threadName;
    int wormsEaten;

    public Chick(Semaphore prod, Semaphore con, Dish dish, String threadName) {
        super(threadName);
        this.prod = prod;
        this.con = con;
        this.dish = dish;
        this.threadName = threadName;
        this.wormsEaten = 0;
    }
    /*
    Each baby bird repeatedly takes a worm, eats it, sleeps for a while, takes another worm, and so on.
    If the dish is empty, the baby bird who discovers the empty dish chirps real loud to awaken the parent bird.

    acquire consumer semaphore in order to consume
    if no worms left in dish, set producer semaphore to 1, and consumer semaphore to 0.
    */

    @Override
    public void run() {
        // TODO

        while(true){
            try{
                // con.tryAcquire();
                con.acquire();
                System.out.println(String.format("%s woke up and is hungry!", threadName));
                //if there are worms to eat
                if(dish.worms > 0){
                    dish.worms--;
                    wormsEaten++;
                    System.out.println(String.format("%s ate a worm, %d worms remaining", threadName, dish.getNumberWorms()));
                    con.release();
                    //Thread.sleep(5000);
                    // random sleep

                    Thread.sleep((int)(10000*Math.random()));
                }
                else{
                    System.out.println(String.format("%s says CHIRP! CHIRP! CHIRP! %d worms in the dish!", threadName, dish.getNumberWorms()));
                    prod.release();
                    // Producer will release consumer semaphore
                }
            }
            catch (InterruptedException ie){
                System.out.println(String.format("Interrupted: %s", threadName));
                System.out.println(ie);
            }
        }
    }
}


// driver class
public class HungryBirds{
    public static void main(String args[]) throws InterruptedException{

        int nChicks, nWorms;
        boolean isFair;

        try{
            if(args.length < 3){
                nChicks = 5;
                nWorms = 12;
                isFair = true;
                System.out.println(String.format("Configured to %d chicks in the nest, Parent Bird puts %d worms in the nest, and fairness is: %b", nChicks, nWorms, isFair));
            }
            else{
                nChicks = Integer.parseInt(args[0]);
                nWorms = Integer.parseInt(args[1]);
                if(args[2].equals("0")){
                    isFair = false;
                }
                else {isFair = true;}

                System.out.println(String.format("Configured to %d chicks in the nest, Parent Bird puts %d worms in the nest, and fairness is: %b", nChicks, nWorms, isFair));
            }

            // initialise shared variables
            Semaphore consumer = new Semaphore(1, isFair);
            Semaphore producer = new Semaphore(1, isFair);
            Dish wormdish = new Dish();
            wormdish.worms = 0;
            System.out.println(String.format("number of worms in dish initially: %d", wormdish.worms));

            ParentBird p = new ParentBird(producer, consumer, wormdish, nWorms, "Birdy parent");
            p.start();
            // a collection of chickens is called either a clutch or a peep
            Chick[] peep = new Chick[nChicks];
            for(int i = 0; i < nChicks; i++){
                peep[i] = new Chick(producer, consumer, wormdish, String.format("chick %d", i + 1));
            }
            for(int i = 0; i < nChicks; i++){
                peep[i].start();
            }


        }

        catch (Exception e){
            System.out.println(e);
        }

    }
}