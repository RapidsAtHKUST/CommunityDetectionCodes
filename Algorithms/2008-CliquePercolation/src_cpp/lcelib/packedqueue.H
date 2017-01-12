/*
lcelib, A high performance library for complex networks research
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef PACKEDQUEUE_H
#define PACKEDQUEUE_H

#include <cassert>
#include <cmath>

#define PREFETCH_DIST 8

/*
 * FIXME: old doc.
 *
 * Templates needed for calendar queue implementations. The event lists are
 * implemeted as linked lists. Due to simplicity and speed considerations
 * (cache misses, single manipulation interface etc.) the lists are
 * singly linked and do not have the usual mock elements at the beginning.
 * Therefore the removal of elements is only possible through the queue 
 * itself. 
 *
 * A basic implementation for events to be put in the queue is given
 * in CalQEvent. Real events can be made by (possibly multiple) inheritance.
 * In addition, by specializing EventTypeTraits it is possible to use
 * any other classes supporting forward-linking and some method
 * for querying time. The time type should be defined in the traits, also.
 * 
 * Only intergral type are supported in this version. The queue is intended
 * for event-based simulations, and using floating-point time representation
 * for such can be generally considered as Bad Idea.
 *
 * This implementation limits the bin sizes and the number of bins
 * to be powers of two. Considering the advantage in the calculation of 
 * the necessary modulo and division operations, this should not be a
 * problem. In addition, the first part of the modulo operation might be 
 * carried out by a cast from unsigned long to unsigned int. 
 *
 * Events are owned by the queues, that is, their destructors do away with 
 * any of them contained. When outside the queue, events are on their own.
 */


template<typename PackedEventType> 
struct PackedEventTypeTraits {
  typedef unsigned long int TimeType;
  static TimeType getEventTime(const PackedEventType & subject) {
    return subject.time;
  }
  static bool eventUsed(const PackedEventType & subject) {
    return subject.isUsed;
  }
  static void markUsageStatus(PackedEventType & subject, bool whetherUsed) {
    subject.isUsed=whetherUsed;
  } 
};

struct TestEvent {
  unsigned long int time;
  bool isUsed;
  TestEvent(unsigned long timeScheduled=0, bool whetherUsed=false):
    time(timeScheduled), isUsed(whetherUsed) {}
};

/* FIXME: old doc.
 * 
 * Functional part of a calendar queue for the given event type. Time type
 * is assumed to be integral and year length less that maximum value
 * of unsigned int (can be assumed to be >= 2^32 = 4G
 */

template<typename PackedEventType> class PackedCalQCore {
private: 
  typedef PackedEventTypeTraits<PackedEventType> MyTraits;
  typedef MyTraits::TimeType TimeType;
  
  /* WHEN ADDING TO BELOW, PLEASE REMEMBER TO MODIFY THE ASSIGNMENT 
   * OPERATOR, ALSO */

  PackedEventType * bins;
  
  unsigned int binSize; 
  unsigned int numBins;

  /* ...and all others redundant with respect to these: */
  
  unsigned int divideShift; // log2 binSize;
  unsigned int moduloMask; // yearLength-1;
  unsigned int logTableSize; 

  /* State variables: */
  
  unsigned int currBin;
  TimeType nextYearStart;
  unsigned int numEvents;
  TimeType lastPopped;
    
  unsigned int getInitSlot(const TimeType time) const { 
    return (time & moduloMask) >> divideShift;
  }

  void advance() {
    currBin=currBin++;
    if (currBin==numBins) {
      currBin=0;
      nextYearStart+=(moduloMask+1);
    }
  }
  
  PackedCalQCore() {};

public:
  PackedCalQCore(unsigned char logBinSize, // To enforce pow2 -rule 
		 unsigned char logNumBins, 
		 TimeType startTime=0) {
    logTableSize=logNumBins;
    binSize = 1 << logBinSize;
    numBins = 1 << logNumBins;
    divideShift=logBinSize;
    moduloMask=binSize*numBins-1; // Year length-1
    lastPopped=startTime;
    currBin=getInitSlot(startTime);
    nextYearStart=((startTime >> (logBinSize + logNumBins))+1)*binSize*numBins;
    bins=new PackedEventType[numBins]; /* Default constr. marks as not used */
    std::cerr << "New queue core. Bins:" << numBins << ", nextYearstart:";
    std::cerr << nextYearStart << ", starttime:" << lastPopped << ",bin size" << binSize << "\n";
    numEvents=0;
  }

  ~PackedCalQCore() {
    delete[] bins;
  }

  PackedCalQCore<PackedEventType> & operator=(PackedCalQCore<PackedEventType>& src) {

    delete[] bins; 
    logTableSize=src.logTableSize;
    binSize=src.binSize;
    numBins=src.numBins;
    divideShift=src.divideShift;
    moduloMask=src.moduloMask;
    currBin=src.currBin;
    nextYearStart=src.nextYearStart;
    numEvents=src.numEvents;
    lastPopped=src.lastPopped;
    bins=src.bins;
    src.bins=0; /* So that we won't delete it. */
    return *this;
  } 

  /**
   * Pushes an event to the queue. We only enforce causality rules here in
   * the assertion. Of course, the causality can be lifted by setting
   * the current bin to the one where a new uncausal one is put and
   * adjusting the lastPopped. This might be useful for some kind of 
   * "lazy" ordering.
   */
  
  void push(PackedEventType & subject) { /* By ref. No copy. */
    PackedEventType tempval;
    unsigned probeLoc=getInitSlot(MyTraits::getEventTime(subject));

    numEvents++;
    
    while (true) {
      //std::cerr << "\nloc:" << probeLoc << "\n";
      while ((MyTraits::eventUsed(bins[probeLoc])) &&
	     (MyTraits::getEventTime(bins[probeLoc]) <= 
	      MyTraits::getEventTime(subject))) {
	assert(eventAtPlace(probeLoc));
	probeLoc=(++probeLoc & (numBins-1));
      }
      if (!MyTraits::eventUsed(bins[probeLoc])) {
	assert(MyTraits::eventUsed(subject));
	bins[probeLoc]=subject;
	return;
      } else {
	assert(MyTraits::getEventTime(bins[probeLoc]) > 
	       MyTraits::getEventTime(subject));
	tempval=bins[probeLoc];
	bins[probeLoc]=subject;
	subject=tempval;
	probeLoc=(++probeLoc & (numBins-1));
      }
    }
  }

  PackedEventType pop(unsigned int & probeLenAcc,
		      unsigned int & numFutEvsAcc) {
    unsigned probeLoc;
    unsigned toBeFilled;
    unsigned initSlot;
    bool probeRotated;
    bool currRotated;

    PackedEventType retval;
    numEvents--;    
    while (!MyTraits::eventUsed(bins[currBin]) ||
	   (MyTraits::getEventTime(bins[currBin]) >= nextYearStart)) {
      /* ...the point being that possible events belonging to 
       * following years are necessarily on the right place.*/
      probeLenAcc++;
      if (MyTraits::eventUsed(bins[currBin])) {
	/* However, we will check that: */
	assert(eventAtPlace(currBin));
	numFutEvsAcc++;
      }
      //std::cerr << "#";
      advance(); 
    }
    //std::cerr << currBin;
    /* Now, we quite necessarily have arrived at the event that
     * should be popped. According to the invariants, it should be at 
     * it's initial place: */
    assert(getInitSlot(MyTraits::getEventTime(bins[currBin]))==currBin);
    /* Unfortunately, we will need a temporary
     * variable: */
    retval=bins[currBin];
    /* We do not mark it as empty, yet. Behold: */

    /* Now, we will have to maintain the queue invariants by possibly 
     * filling this place. All the immediately following bins are 
     * necessarily in the right order. */
    toBeFilled=currBin;
    probeLoc=((currBin+1) & (numBins-1));
    while (MyTraits::eventUsed(bins[probeLoc])) {
      initSlot=getInitSlot(MyTraits::getEventTime(bins[probeLoc]));  
      currRotated=(initSlot>probeLoc);
      probeRotated=(probeLoc<toBeFilled);
      /* You are not expected to understand this: */
      if (((probeRotated==currRotated) &&
	   (initSlot <= toBeFilled)) ||
	  (currRotated && !probeRotated)) {      
	bins[toBeFilled]=bins[probeLoc];
	assert(eventAtPlace(toBeFilled));
	toBeFilled=probeLoc;
      }
      probeLoc=(++probeLoc & (numBins-1));
      /* ...and now, we did not succeed in filling the place */
    }
    MyTraits::markUsageStatus(bins[toBeFilled], false);
    return retval;
  }

  void consume(PackedCalQCore<PackedEventType> & source) {
    unsigned int i=0;
    for (i=0; i<source.getNumBins(); i++)
      if (MyTraits::eventUsed(source.bins[i])) {
	push(source.bins[i]);
      }
  }

  /**
   * There is not a method for querying the minimum time, because we 
   * cannot advance the currBin pointer beyond the last event 
   * popped: another with the same time stamp could be pushed soon.
   */
  TimeType getCurrTime() const {return lastPopped;} 
    
  TimeType getYearLength() const {return moduloMask+1;}
  unsigned getLogBinSize() const {return divideShift;}
  unsigned getLogNumBins() const {return logTableSize;}

  unsigned getNumEvents() const {return numEvents;}

  unsigned getNumBins() const {return numBins;}

  
  void printout(int limit=0) const {
    std::cerr << "\n";
    if ((limit>numBins) || (limit==0)) limit=numBins;
    for (int i=0; i<limit; i++) {
      std::cerr << i << MyTraits::eventUsed(bins[i]);
      if (MyTraits::eventUsed(bins[i])) {
	std::cerr << " " << MyTraits::getEventTime(bins[i]) << " " 
		  << getInitSlot(MyTraits::getEventTime(bins[i]));
      }
      std::cerr << "\n";
    }
  }
  
  bool eventAtPlace(unsigned place) {
    unsigned probeLoc=getInitSlot(MyTraits::getEventTime(bins[place]));
    while (MyTraits::eventUsed(bins[probeLoc])) {
      if (probeLoc==place) return true;
      /* If we do not want to find the element, we do not check the order */
      else if (MyTraits::eventUsed(bins[place]) && // The queried one...
	       (MyTraits::getEventTime(bins[probeLoc]) >
		MyTraits::getEventTime(bins[place]))) return false;
      probeLoc=(++probeLoc & (numBins-1));
    }
    return false;
  }

  bool isLegal() {
    unsigned numEvsFound=0;
    for (int i=0; i<numBins; i++) {
      if (MyTraits::eventUsed(bins[i])) numEvsFound++;
      /* If in use, must be found, otherwise not: */
      if (eventAtPlace(i) != MyTraits::eventUsed(bins[i])) return false;
    }
    return (numEvsFound==numEvents);
  }
  

};
   
/**
 * A dynamic calendar queue implementation inpired by articles by ? and ?.
 * The basic assumption made is that the highest density of events is
 * found at the current time. This should be approximately true
 * for any event-generating mechanisms with no expilicit time dependence. 
 * 
 * In order to avoid linked list traversal, we aim at having approximately
 * two bins per event at current time. We use exclusively costs associated 
 * with pop operations in order to trigger requeue operations. The metrics
 * gathered are: 
 * 
 * 1. The density D of events belonging to the current year. 
 *
 * 2. The average number of events not belonging to current year encountered
 *    during probe: F.
 * 
 * The D is used to calculate the optimal bin size, whereas F is used for the
 * optimal year length.
 *
 *
 */

template<typename PackedEventType> 
class MyPackedCalQueue { 
  typedef PackedEventTypeTraits<PackedEventType>::TimeType TimeType;
private:
  PackedCalQCore<PackedEventType> queue;
  unsigned int popProbeLenSum; 
  unsigned int popFutureEventSum;
  unsigned int popCounter;
  unsigned int fillLimit;
  
  void requeue(const int binSizeLogChange, 
	       const int numBinsLogChange) {
    PackedCalQCore<PackedEventType> 
      newQueue(queue.getLogBinSize()+binSizeLogChange, 
	       queue.getLogNumBins()+numBinsLogChange,
	       queue.getCurrTime());
    assert(queue.isLegal());
    std::cerr << "\nRequeue. Former log bins:" << queue.getLogNumBins()
	      << ", change:" << numBinsLogChange << "\n";
    newQueue.consume(queue);
    assert(newQueue.isLegal());
    assert(newQueue.getNumEvents()==queue.getNumEvents());
    queue=newQueue; /* Automagic deletion of bins. */
    popProbeLenSum=0;
    popFutureEventSum=0;
    popCounter=0;
    fillLimit=(1 << (queue.getLogNumBins()-1));
  }

public:
  MyPackedCalQueue(TimeType startTime=0, unsigned initLogSize=8):
    queue(0, initLogSize, startTime), popProbeLenSum(0), 
    fillLimit(1 << (initLogSize-1)), 
    popFutureEventSum(0), popCounter(0) {}
  
  unsigned int push(PackedEventType newEvent) {
    if (queue.getNumEvents() == fillLimit) {
      requeue(0, 1);
    }
    queue.push(newEvent);
    return queue.getNumEvents();
  }

  PackedEventType pop() {
    
    if (popCounter == fillLimit) { /* Time to check */      
      std::cerr << "Checking for resize. Probe len avg:";
      std::cerr << ((float) popProbeLenSum/fillLimit) << ", future avg:";
      std::cerr << ((float) popFutureEventSum/fillLimit) << "\n"; 
            
      int numBinsLogChange=0;
      int yearLenLogChange=0;
      int binSizeLogChange=0;
      int logDenom=(queue.getLogNumBins()-1);
      assert((1 << logDenom) == fillLimit);      
      /* Bin size -loop. Aim to probe length 4. */
      while ((popProbeLenSum >> (logDenom + binSizeLogChange + 1)) == 0) {
	binSizeLogChange--;
      }
      /* Up: */
      while ((popProbeLenSum >> (logDenom + binSizeLogChange + 1)) > 3) { 
	binSizeLogChange++;
	std::cerr << "Bin size up. Comp value" 
		  << (popProbeLenSum >> (logDenom + binSizeLogChange + 1))
		  << "\n";	  
      }

      /* Year length -loop. Down: */
      while ((popFutureEventSum >> (logDenom-2+yearLenLogChange)) == 0) {
	yearLenLogChange--;
      }
      while ((popFutureEventSum >> (logDenom-2+yearLenLogChange)) > 3) {
	yearLenLogChange++;
      }
      std::cerr << "Y len log ch:" << yearLenLogChange << ",bin sz log ch:"
		<< binSizeLogChange << "\n";
      numBinsLogChange=yearLenLogChange-binSizeLogChange;
      
      if (binSizeLogChange || numBinsLogChange) {
	std::cerr << "Calqueue resize. Bin size:" 
		  << (1 << (queue.getLogBinSize()+binSizeLogChange))
		  << ", numBins:"
		  << (1 << (queue.getLogNumBins()+numBinsLogChange)) << "\n";
	requeue(binSizeLogChange, numBinsLogChange);
      } else {
	popProbeLenSum=0;
	popFutureEventSum=0;
	popCounter=0;
      }
      
    }
    popCounter++;
    return queue.pop(popProbeLenSum, popFutureEventSum);     
  }
  unsigned getNumEvents() {return queue.getNumEvents();}
  void printout(int i=100) {queue.printout(i);}
};

  
#endif





