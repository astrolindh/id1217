/*
ID1217 Parallel programming, KTH, VT2022
Astrid Lindh

Problem 5 - recreate the hungry Birds problem without the use of semaphores

"Develop a monitor (with condition variables) to synchronize the actions of the birds, i.e. develop a monitor that
represents the dish. Define the monitor's operations and their implementation. Implement a multithreaded application
in Java or C++ to simulate the actions of the birds represented as concurrent threads and the dish represented as the
developed monitor. Is your solution fair? Explain in comments in the source code."

 */

import java.util.concurrent.Semaphore;


// TODO bygg ut Dish som en monitorklass
// variabeldeklaration (state)
// initialiseringskod, konstruktorn?
// procedurer: alltså metoder och operatoioner. Ska vara synchronized
class Dish
{
    private int currentWorms, maxWorms;
    // Condition not_full; // condition variable är implicit, genom användan av wait, notify, notyAll
    // Condition not_empty;
    //private Object[] buffer;


    public Dish(int startingWorms, int maximumWorms){
        this.currentWorms = startingWorms;
        this.maxWorms = maximumWorms;
        // buffer = new Object[n];
    }

    // called by producers
    public synchronized void deposit(){
        // if dish is filled up
        try{
            while(currentWorms > 0){
                this.wait();
            }
            // else, deposit a load of womrs
            currentWorms = maxWorms;
            notifyAll();
        }
        catch(InterruptedException ie){
            System.out.println(ie);
        }
    }

    // called by consumers
    public synchronized void consume(){
        try{
            // wait for dish to fill up
            while(currentWorms == 0){
                wait();
            }
            // eat a worm
            currentWorms--;
            // System.out.println(String.format("%d worms in the dish", currentWorms));
            notify();
        }
        catch(InterruptedException ie){
            System.out.println(ie);
        }
    }
}


// class ParentBird - the single Producer
class ParentBird extends Thread {
    // BinarySem prod, con;
    Dish dish;
    int nWorms;
    String threadName;

    public ParentBird(Dish dish, int nWorms, String threadName) {
        super(threadName);
        this.dish = dish;
        this.nWorms = nWorms;
        this.threadName = threadName;
    }

    @Override
    public void run() {
        while(true){
            try{
                dish.deposit();
                System.out.println(String.format("SQUAWK! %s put %d worms in the dish", threadName, nWorms));
            }
            catch (Exception ie){
                System.out.println("Interruption in ParentBird");
                System.out.println(ie);
            }
        }
    }
}

class Chick extends Thread {
    // BinarySem prod, con;
    Dish dish;
    String threadName;
    int wormsEaten;

    public Chick(Dish dish, String threadName) {
        super(threadName);

        this.dish = dish;
        this.threadName = threadName;
        this.wormsEaten = 0;
    }

    @Override
    public void run() {
        while(true){
            try{
                dish.consume();
                wormsEaten++;
                System.out.println(String.format("%s ate a worm, %d worms eaten total", threadName, wormsEaten));
                Thread.sleep((int)(10000*Math.random()));
            }
            catch (InterruptedException ie){
                System.out.println(String.format("Interrupted: %s", threadName));
                System.out.println(ie);
            }
        }
    }
}

// driver class
public class MonitorBirds{
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

            Dish wormdish = new Dish(0, nWorms);

            ParentBird p = new ParentBird(wormdish, nWorms, "Birdy parent");
            p.start();

            Chick[] peep = new Chick[nChicks];
            for(int i = 0; i < nChicks; i++){
                peep[i] = new Chick(wormdish, String.format("chick %d", i + 1));
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