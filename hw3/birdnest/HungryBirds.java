import java.util.concurrent.*;

// class Dish of Worms. Should be protected by a shared semaphore
class Dish
{
    static int worms = 0;
}

// class producent Parent
class Parent extends Thread {
    Semaphore sem;
    String threadName;

    public Parent(Semaphore sem, String threadName) {
        super(threadName);
        this.sem = sem;
        this.threadName = threadName;
    }

    @Override
    public void run() {
        // TODO
    }
}

// class consumer Chick
class Chick extends Thread {
    Semaphore sem;
    String threadName;

    public Chick(Semaphore sem, String threadName) {
        super(threadName);
        this.sem = sem;
        this.threadName = threadName;
    }

    @Override
    public void run() {
        // TODO
    }
}


// driver class
public class HungryBirds{
    public static void main(String args[]) throws InterruptedException{
        // take N and make N Chicks
        // take W and make W the number of worms
        // make 1 Parent
        // make 1 Semaphore

        // while forever, run threads

    }
}