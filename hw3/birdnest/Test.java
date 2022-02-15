import java.util.concurrent.*;

class Shared {
    static int count = 0;
}

// en task?
class Thready extends Thread{
    Semaphore sem;
    String threadName;
    public Thready(Semaphore sem, String threadName){
        super(threadName);
        this.sem = sem;
        this.threadName = threadName;
    }

    @Override
    public void run(){
        // conditioned to only accept at thread called A
        if(this.getName().equals("A")){
            System.out.println("starting " + threadName);
            try{
                // thread waits for a permit
                System.out.println(threadName + " is waiting for a permit");
                sem.acquire();
                System.out.println(threadName + " acquired a permit");

                for(int i = 0; i < 5; i++){
                    Shared.count++;
                    System.out.println(threadName + " sets counter to: " + Shared.count);
                    Thread.sleep(3);
                }

            }
            catch (InterruptedException exc){
                System.out.println(exc);
            }
            // release permit
            System.out.println(threadName + " releases a permit");
            sem.release();

        }
        else{
            System.out.println("starting " + threadName);
            try{
                System.out.println(threadName + " is waiting for a permit");
                sem.acquire();
                System.out.println(threadName + " acquired a permit");

                for(int i = 0; i < 5; i++){
                    Shared.count--;
                    System.out.println(threadName + " sets counter to: " + Shared.count);
                    Thread.sleep(3);
                }
            }
            catch (InterruptedException exc){
                System.out.println(exc);
            }
            System.out.println(threadName + " releases a permit");
            sem.release();
        }
    }
}

// the driver
public class Test{
    public static void main(String args[]) throws InterruptedException {
        Semaphore sem = new Semaphore(1);
        Thready t1 = new Thready(sem, "A");
        Thready t2 = new Thready(sem, "B");

        t1.start();
        t2.start();

        t1.join();
        t2.join();

        System.out.println("shared counter is " + Shared.count);
    }
}