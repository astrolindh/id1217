

import java.util.concurrent.Semaphore;

public class Dish
{
    private int worms, capacity;

    public Dish(int startWorms, int capacity){
        this.worms = startWorms;
    }
    synchronized public int getNumberWorms(){ return worms; }
    synchronized public void setNumberWorms(int newWorms) {worms = newWorms;}

}

public class BinarySem extends Semaphore{
    //    private static final long serialVersionUID = -927596707339500451L;
    private final Object lock = new Object; // a dummy object,

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
            if(this.avaialablePermits() == 0)   super.release();
            else                                System.out.println("attempted releasing released permit");
        }
    }
}