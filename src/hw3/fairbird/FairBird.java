

import java.util.concurrent.Semaphore;

class Dish
{
    private int worms;

    public Dish(int startWorms){
        this.worms = startWorms;
    }
    synchronized public int getNumberWorms(){ return worms; }
    synchronized public void setNumberWorms(int newWorms) {worms = newWorms;}
    synchronized public boolean isEmpty(){
        if(worms < 1) return true;
        return false;
    }
    synchronized public void eatWorm() { worms--; }
    synchronized public void addWorms(int moreWorms) { worms += moreWorms; }
}

class BinarySem extends Semaphore{
    //    private static final long serialVersionUID = -927596707339500451L;
    private final Object lock = new Object(); // a dummy object,

    public BinarySem(boolean startReleased, boolean isFair){
        super((startReleased ? 1: 0), isFair);          // creates a new Semaphore(startReleased number of permits, with or without FIFO queue
    }

    /*
    acquire the BinarySemaphore permit
     */
    @Override
    public void acquire() throws InterruptedException {
        super.acquire(1);
    }

    /*
    release the BinarySemaphore permit
     */
    @Override
    public void release(){
        synchronized (lock){
            if(this.availablePermits() == 0){ super.release(); }
            else                                System.out.println("attempted releasing released permit");
        }
    }
}

// class ParentBird - the single Producer
class ParentBird extends Thread {
    BinarySem prod, con;
    Dish dish;
    int nWorms;
    String threadName;

    public ParentBird(BinarySem prod, BinarySem con, Dish dish, int nWorms, String threadName) {
        super(threadName);
        this.prod = prod;
        this.con = con;
        this.dish = dish;
        this.nWorms = nWorms;
        this.threadName = threadName;
    }

    @Override
    public void run() {
        while(true){
            try{
                prod.acquire();
                System.out.println("Parent bird has woken up!");
                dish.addWorms(nWorms);
                System.out.println(String.format("Parent bird refilled the dish with %d worms, now there's %d worms in the dish", nWorms, dish.getNumberWorms()));
                con.release();
            }
            catch (InterruptedException ie){
                System.out.println("Interruption in ParentBird");
                System.out.println(ie);
            }
        }
    }
}

class Chick extends Thread {
    BinarySem prod, con;
    Dish dish;
    String threadName;
    int wormsEaten;

    public Chick(BinarySem prod, BinarySem con, Dish dish, String threadName) {
        super(threadName);
        this.prod = prod;
        this.con = con;
        this.dish = dish;
        this.threadName = threadName;
        this.wormsEaten = 0;
    }

    @Override
    public void run() {
        while(true){
            try{
                con.acquire();
                if(!dish.isEmpty()){
                    dish.eatWorm();
                    wormsEaten++;
                    System.out.println(String.format("%s ate a worm, %d worms total", threadName, wormsEaten));
                    con.release();
                }
                else{
                    System.out.println(String.format("%s says CHIRP! CHIRP! CHIRP! %d worms in the dish!", threadName, dish.getNumberWorms()));
                    prod.release();
                }
                Thread.sleep((int)(10000*Math.random()));
                //Thread.sleep(3000);
            }
            catch (InterruptedException ie){
                System.out.println(String.format("Interrupted: %s", threadName));
                System.out.println(ie);
            }
        }
    }
}

// driver class
public class FairBird{
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

            BinarySem consumer = new BinarySem(true, isFair);
            BinarySem producer = new BinarySem(true, isFair);
            Dish wormdish = new Dish(0);
            System.out.println(String.format("number of worms in dish initially: %d", wormdish.getNumberWorms()));

            ParentBird p = new ParentBird(producer, consumer, wormdish, nWorms, "Birdy parent");
            p.start();

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