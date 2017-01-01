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

#ifndef LCE_NNET
#define LCE_NNET

#include"../Containers/Indices/LinearHash.H"
#include"../Containers/Tables/ValueTable.H"
#include"../eventsim/QalQueue.H"
#include"../eventsim/EventBase.H"
#include<algorithm>


/*
 * An action potential, the primary object-oriented subject in neural network
 * simulations. Can be parametrized as either virtual or non-virtual, 
 * depending on whether functionally different synapses exist in the network.
 *
 * RATIONALE: no virtual function calls have to be performed unless needed. 
 * In basic event-driven operations, the commit-call can be the only 
 * virtual function called when processing an event: after it, the type
 * of every associated class is known exactly. 
 * 
 * @param Network   The type of the network itself, typedef-ing the  TimeType
 * @param Synapse   The non-virtual synaptic class included. Accessed 
 *                  primarily virtually through this class, however.
 * @param EventBase Base allowing for parametrization of the virtuality of
 *                  the commit-op
 */

template<typename DelayType=unsigned, typename StrengthType=float>
class SynBase {
protected:
  DelayType xmitDelay;
  unsigned target;
  StrengthType strength;
public:
  SynBase(unsigned tgt, DelayType delay=0, StrengthType str=0): 
    xmitDelay(delay), target(tgt), strength(str) {} 
  unsigned key() {return target;} /* Immutable */
  StrengthType & value() {return strength;}
  DelayType delay() {return xmitDelay;}
}

template<typename TimeType typename DelayType=unsigned, 
	 typename StrengthType=float>
class STDPSynBase: public SynBase<DelayType, StrengthType> {
  



  

public:
  operator unsigned() {return target;}
  TimeType getDelay() {return delay;}
};

template<typename Synapse>
class SynSorter {
public:
  bool operator()(Synapse & first, Synapse & second) {
    return first.getDelay()<second.getDelay();
  }
};

class ActPotBase<>

/**
 * This should allow for copying of neuron _types_ in a portable 
 * manner. IsVirtual is true here; partial specialization exists for the 
 * false (non-virtual) case. Uses EBCO: RTFM.
 */

template<bool IsVirtual, typename Synapse, typename EventBase> 
class AxonBase {
  typedef AxonBase<true, SynBase, EventBase> MyType;
 
  

  virtual SynBase & operator[](size_t i) {
    assert(false);
  }

  virtual const SynBase & operator[](size_t i) const {
    assert(false);
  }

  virtual void setSynAt(size_t i, SynBase & src) {
    assert(false);
  }

  virtual EventBase * zap(TimeType time) {
    assert(false);
  }

  virtual void postSynZap(SynBase * target) {
    assert(false);
  }

};



/**
 * Non-virtual case. Uses EBCO. RTFM.
 */

template<typename SynType, typename EventBase>
class AxonBase<false> {
  void getTypeFrom(AxonBase<false> & src) {}
};

class NeuronBase<bool isVirtual> {
  
}

template<typename Synapse, bool > 
class NeuronBase {
protected:
  Synapse * axon=null;
  unsigned numSynapses;
public:



  void assemble() {
    std::sort(axon, &axon[numOut], SynSorter<Synapse>());
  }

  size_t size() const {return numSynapses;}

  

};



  class ActPot::public NetworkType::EventBase {
  protected:
    Synapse * curr;
    Synapse * end;
    
  public:
 
    ActPot(Synapse * begin, Synapse * _end, TimeType time): 
      super(time+begin->time), curr(begin), end(_end) {}  

    /* Virtuality is given by the EventBase. */
    void commit(NetworkType & context,
		EventQueue & queue) {
      do {
	/* This is definitely not virtual anymore. */
	curr->zap(time, Network, queue);
	time-=curr->getDelay();
	curr++;
	if (curr==end) {
	  delete this;
	  return;
	} 
	assert(curr->getDelay() <= (curr-1)->getDelay());
	time+=curr->getDelay();
      } while (time <= queue.nextEventTime);
      this.setPrefetchPointer(Network.getTargetPointer(curr->target));
      queue.push(this);
    }
  };

  ActPot * zap() {
    return new ActPot(axon, &axon[numOut], time);
  }

  void postSynZap(TimeType time, Synapse::Base * loc) {
    /* Is it for me to zap? */
    assert((loc >= axon) && (loc < &(axon[numSynapses])));
    /* Won't believe yet. Gotta really point into a synapse */
    assert((((void *) loc)-((void *) axon))%sizeof(SynType) == 0);
    /* Gotta believe */
    ((Synapse *) loc).postZap(time); 
  }


template<typename SynBase, NeuronBase, 

template<typename TimeType> 
class IFSoma {
  typedef EventQueue::
  float lastVoltage;
  TimeType lastTime;
  TimeType lastFire;
public:
  static const float lambda;
  static const TimeType refract;
  void update(TimeType when) {
    lastVoltage*=exp((when-lastTime)*lambda);
    lastTime=when;
  }
  TimeType zap(const float strength, const TimeType time) {
    if (time > lastFire+refract) {
      update(time);
      lastVoltage+=strength;
      if (lastVoltage > 1) {
	lastVoltage=0;
	lastFire=time;
      }
    }
    return time-lastFire;
  }
};

template<template<typename TimeType> class SomaTemplate, 
	 template<typename TimeType> class SynBaseTemplate,
	 bool IsVirtual=true,
	 typename _TimeType=unsigned,
	 template<typename TimeType> class EvQueueTemplate=CalQueue>
class SpikeNet {
public:
  typedef _TimeType TimeType;
  typedef Soma<TimeType> Soma;
  typedef SynBaseTemplate<_TimeType> SynBase;
  typedef EvQueueTemplate<_TimeType> EventQueue;
  typedef SpikeNet<SomaTemplate, SynBaseTemplate, IsVirtual, 
		   TimeType, EvQueueTemplate> MyType;
  typedef Axon<SynBase, IsVirtual> AxonType;
  typedef AutoMap<size_t, SynBase *> Dendritic;
  

protected:
  size_t numNeurons;
  Soma * somae;
  AxonType * axons;
  Dendritic * dendrites;

private:
  SpikeNet() {} /* Def. Not to be used. */
public:

  SpikeNet(const size_t size) {
    numNeurons=size;
    somae=new Soma[numNeurons];
    axons=new AxonType[numNeurons];
    dendrites=new Dendritic[numNeurons];
  }

  ~SpikeNet{
    delete[] axons;
    delete[] dendrites;
    delete[] somae;
  }

public:

  class Neuron {
  protected:
    MyType & network;
    size_t neurIndex;
  public:
    void setSynType:
  }:
  

  class ConstNeuron {
    My
  }

  typedef Neuron reference;

  template<typename SourceNetType> 
  SpikeNet(SourceNetType & src) {
    numNeurons=size;
    somae=new Soma[numNeurons];
    axons=new AxonType[numNeurons];
    dendrites=new Dendritic[numNeurons];  
    for (SourceNetType::iterator i=src.begin; !i.finished(); ++i) {
      
      reference curr=this->operator[](*i);
      curr.resize(i->size());
      size_t num=0;
      for (SourceNetType::edge_iterator j=reference.begin(); 
	   !j.finished; ++j) {
	(&curr)->putAt(num, *j, j.value());
	j.remove(); /* We consume it. */
      }
      (&curr)->sort();
      for (reference::iterator j=curr.begin(); !curr.finished(); ++curr) {
	(&(operator[](*j)))->incoming[*j]=
      }    
    }
  }
  
 } 




#endif
