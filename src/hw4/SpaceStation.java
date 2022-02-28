/*
Consider a future fuel station in space that supplies nitrogen and quantum fluid. Assume that the station has a
capacity of handling V space vehicles in parallel (simultaneously) and a maximum storage for N liters of nitrogen and
for Q liters of quantum fluid. When a vehicle arrives at the station it requests the needed amount of fuel of the two
different types or only one of the types. If there is not enough fuel of either type, the vehicle has to wait without
blocking other vehicles. Special supply vehicles deliver fuel in fixed quantities that far exceed a fuel tank capacity
of any vehicle. When a supply vehicle arrives at the station, it delays until there is enough space to deposit the
delivered amount of fuel. To travel back, the supply vehicles also request a certain amount of fuel of the two
different types or one of the types just like an ordinary vehicle, not necessarily the type it supplies.

Develop and implement a multithreaded application (in Java or C++) that simulates the actions of the ordinary space
vehicles and supply vehicles represented as concurrent threads. Your simulation program should implemented all
"real world" concurrency in actions of the vehicles as described in the above scenario. Represent the fuel space
station as a monitor (a synchronized object) containing a set of counters that define the amounts of available fuel of
different types (at most  N liters of nitrogen and at most Q liters of quantum fluid) and the number of free docking
places (at most V places). The monitor should be used to control access to and the use of the station. Develop and
implement the monitor's methods. The vehicle threads calls the monitor methods to request and release  access to the
station in order to get or/and deposit fuel.

In your simulation program, assume that each vehicle periodically arrives at the station to get/supply fuel.
Have the vehicles sleep (pause) for a random amount of time between arriving at the station to simulate the time it
takes to travel in space; and have the vehicles sleep (pause) for a smaller random amount of time to simulate the time
it takes to get/supply the fuel at the station. Stop the simulation after each vehicle has arrived at the station trips
(the given amount) times. Your program should print a trace of the interesting events in the program.
Is your solution fair? Explain when presenting homework.
 */

import java.util.concurrent.*;

//Monitor class
// Monitor invariant is a predicate about a monitor state that is true when no call is active.
class FuelStation{
    // Q -quantum fuel. capacity and current stock/available stock
    // N - nitrogen. capcity and current stock
    // V - number of space vehicles. capacity and current number being served (or waiting?)
    int stockedQ, stockedN, capQ, capN, nV;

    public Fuelstation(int startingQ, int startingN, int capacityQ, int capacityN, int V){
        this.stockedQ = startingQ;
        this.stockedN = startingN;
        this.capQ = capacityQ;
        this.capN = caqpacityN;
        this.nV = V;
    }


    public synchronized void deliverFuel(int depositQ, int depositN){
        // When a supply vehicle arrives at the station, it delays until there is enough space to deposit the
        //delivered amount of fuel. To travel back, the supply vehicles also request a certain amount of fuel of the two
        //different types or one of the types just like an ordinary vehicle, not necessarily the type it supplies.

    }

    public synchronized void consumeFuel(int neededQ, int neededN){
        /*When a vehicle arrives at the station it requests the needed amount of fuel of the two different types or
        only one of the types. If there is not enough fuel of either type, the vehicle has to wait without blocking
        other vehicles.*/
    }
}

class Vehicle implements Runnable {
    int tripsMax, tripsTaken;
    int needsQ, needsN;

    @Override
    public void run(){
        try{
            while(tripsTaken < tripsMax){
                // TODO:
                // arrive at station
                // fill up
                // increment trips taken
                // random thread sleep,
            }
            // thread.exit eller nåt
        }
        catch(Exception ie){System.out.println(ie);}
    }

}

class SupplyVehicle implements Runnable {
    int tripsMax, tripsTaken;
    int tankQcap, tankNcap;
    int needsQ, needsN;

    @Override
    public void run(){
        try{
            while(tripsTaken < tripsMax){
                // TODO:
                // arrive at station
                // deliver fuel
                // fill up
                // increment trips taken
                // random thread sleep,
            }
            // thread.exit eller nåt
        }
        catch(Exception ie){System.out.println(ie);}
    }
}



public class SpaceStation
{
    public static void main(String args[]){
        // read given variables
        // initialize
        //run
        }
}