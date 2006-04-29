/* -*- C++ -*-

   This file implements the Weaver class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: ThreadWeaver.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <QMutex>
#include <QMutexLocker>

#include "ThreadWeaver.h"
#include "WeaverObserver.h"
#include "WeaverImpl.h"

namespace ThreadWeaver {

    /*! \mainpage Introduction to ThreadWeaver

      \section UseCases Use Cases

      <p>ThreadWeaver provides a solution to a number (but not all)
      multithreading problems. Let's have a look:</p>

      <ul> <li>How do you implement a single operation that takes a lot of CPU
      power, but is hard to handle in chunks (Example: scale an image) when
      you want your GUI to remain responsive? Encapsulate the operation in a
      class object derived from Job instead of a method and put it in the
      static instance (Weaver::instance). Connect to the Job's done() signal
      to receive a notification when it is completed.</li>

      <li>How do you implement a CPU intensive operation while time-critical
      operations are executed (load and decode an MP3 while another one is fed
      to the audio device): Implement both the file loading operation and the
      play operation in a job and queue both at the same time. There is also a
      synchronous sleep method in the Job class if a job needs to be delayed
      for a number of millieconds after it is taken over by a thread.</li>

      <li>How do you implement many small operations where the cost of each
      individual one is hard to estimate (loading two hundred icons from an
      NFS drive): Create a common or a number of specialized job
      classes. Queue all of them. Either connect to the individual done signal
      to process every item when it has been finished or connect to
      Weaver::jobsDone() to receive a notification when all of the jobs are
      done.</li>

      <li>How do you implement an operation with complex control flow and
      dependencies in the execution order (load, parse and display an HTML
      document with embedded media): Create jobs for the individual steps you
      need to perform. Try to split the whole operation in as many
      independent, parallelizable parts as possible. Now declare the execution
      dependencies. A job will only be executed when all jobs it depends on
      are finished. This way every individual operation will be executed as
      soon as it becomes possible. Connect to the final jobs done() signal to
      be notified when all parts of the whole operations have been
      executed. If necessary (if there is more than one fina object), create a
      dummy job object that depends on all of them to have one central end
      point of execution.</li> </ul>

      <p>As you can see, ThreadWeaver can provide solutions for simple, but
      also for complex cases. For an example on how job dependencies can be
      modeled and used to create elegant, streamlined solutions for control
      flow modeling, see the Simple Multithreaded Image Viewer (SMIV) example
      in the Tests directory.</p>

      <p>ThreadWeaver focuses on operations that can be implemented in
      Jobs. To create solutions where a thread is supposed to constantly run
      to perform an ongoing operation that, for example, spans the whole
      application live, it has to be verified that it is the right
      approach. It is possible to add very long or neverending operations to
      the queue, but it will constantly bind a thread to that operation. It
      might still make sense to use ThreadWeaver, as the creation, handling
      and destruction of the threads is already taken care of, but the minimum
      inventory size (thread count) should be increased accordingly to provide
      for enough threads to process the other jobs.</p>

      \section Why Why Multithreading?

      <p>In the past, multithreading has been considered a powerful tool that
      is hard to handle (some call it the work of the devil). While there may
      be some truth to this, newer tools have made the job of the software
      developer much easier when creating parallel implementations of
      algorithms. At the same time, the necessity to use multiple threads to
      create performant applications has become more and more
      clear. Technologies like Hyperthreading and multiple core processors can
      only be used if processors have to schedule processing power to
      multiple, concurrently running processes or threads.</p>

      <p>Event driven programs especially bear the issue of latency, which is
      more important for the user's impression of application performance than
      other factors. But the responsiveness of the user interface relies
      mainly on the ability of the application to process events, an ability
      that is much limited in case the application is executing processing
      power expensive, lengthy operations. This leads, for example, to delayed
      or sluggish processing of necessary paint events. Even if this does not
      at all influence the total time necessary to perform the operation the
      user requested, is is annoying and not state of the art. </p>

      <p>There are different approaches to solve this issue. The crudest one
      may be to process single or multiple events while performing a lengthy
      operation, which may or may not work sufficiently, but is at least sure
      to ruin all efforts of Separation of Concerns. Concerns can simply not
      be separated if the developer has to intermingle instructions with event
      handling where he does not even know about the kind of events that are
      processed. </p>

      <p>Another approach is to use event-controlled asynchronous
      operations. This is sufficient in most cases, but still causes a number
      of issues. Any operation that carries the possibility of taking a long
      time or blocking may still stop event processing for a while. Such risks
      are hard to assess, and especially hard to test in laboratory
      environments where networks are fast and reliable and the system I/O
      load is generally low. Network operations may fail. Hard disks may be
      suspended. The I/O subsystem may be so busy that transfering 2 kByte may
      take a couple of seconds. </p>

      <p>Processing events in objects that are executed in other threads is
      another approach. It has it's own issues that come with parallel
      programming, but it makes sure the main event loop returns as soon as
      possible. Usually this approach is combined with a state pattern to
      synchronize the GUI with the threaded event processing. </p>

      <p>Which one of these approaches is suitable for a specific case has to
      be assessed by the application developers. There is no silver
      bullet. All have specific strengths, weaknesses and issues. The
      ThreadWeaver library provides the means to implement multithreaded job
      oriented solutions. </p>

      <p>To create performant applications, the application designers have to
      leverage the functionality provided by the hardware platform as good as
      possible. While code optimizations only lead to so much improvement,
      application performance is usually determined by network and I/O
      throughput. The CPU time needed is usually negligible. At the same time,
      the different hardware subsystems usually are independent in modern
      architectures. Network, I/O and memory interfaces can transfer data all
      at the same time, and the CPU is able to process instructions while all
      these subsystems are busy. The modern computer is not a traditional
      uniprocessor (think of GPUs, too). This makes it necessary to use all
      these parallel subsystems at the same time as much as possible to
      actually use the possibilities modern hardware provides, which is very
      hard to achieve in a single thread.</p>

      <p>Another very important issue is application processing
      flow. Especially GUI applications to not follow the traditional
      imperative programming pattern. Execution flow is more network-like,
      with chunks of code that depend on others. Tools to represent those
      networks to set up your applications order of execution are rare, and
      usually leave it to the developers to code the execution order of the
      instructions. This solutions are usually not flexible and do not adapt
      to the actual usage of the CPU nodes and computer
      subsystems. ThreadWeaver provides means to represent code execution
      dependencies and relies on the operating systems scheduler to actually
      distribute the work load. The result is an implementation that is very
      close to the original application semantics, and usually improved
      performance and scalability in different real-life scenarios. </p>

      <p>The more tasks are handled in parallel, the more memory is
      necessary. There is a permanent CPU - memory tradeoff which limits the
      number of parallel operations to the extend where memory that needs to
      be swapped in and out slows down the operations. Therefore memory usage
      needs to be equalized to allow the processors to operate without being
      slowed down. This means parallel operations need to be scheduled to a
      limit to balance CPU and memory usage. ThreadWeaver provides the means
      for that. </p>

      <p>In general, ThreadWeaver tries to make to task of creating
      multithreaded, performant applications as simple as
      possible. Programmers should be relieved of synchronization, execution
      dependendancy and load balancing issues as much as possible. The API
      tends to be clean, extensible and easy to understand. </p>


      \section MainComponents Main Components of ThreadWeaver

      <p>ThreadWeaver is a Job queue. It executes jobs in threads it
      internally manages. The minimum and maximum number of threads provided
      by a Weaver is set by the user. Jobs are regular QObjects, which allows
      users to connect to the done() signal to be notified when the Job has
      been executed. The Weaver class provides objects that handle a number of
      threads called the inventory. Users usually acquire a reference to a
      WeaverInterface object. </p>

      <p>Jobs may depend on other jobs. A job will only execute if all jobs it
      depends on are already finished. In this, dependencies reorder job
      execution.  If no dependencies are declared, jobs are executed in
      queueing order. Multiple dependencies are possible, which allows the
      creation of complex flow graphs that are automatically executed by the
      Weaver. It is important, though, to avoid circular dependencies. Two
      jobs that depend on each other in both directions will simply never be
      executed, since the dependencies will never resolve.</p>

      <p>Threads are created on demand and do not exit until the containing
      weaver is deleted. Threads have an eager policy in trying to execute
      jobs out of the queue. The managing Weaver blocks them if no jobs are
      availabe.</p>

      <p>WeaverObservers are used to receive more informative events about the
      thread states and job execution. They can be used to provide progress or
      debugging information or to implement GUIs on the thread
      activity. Observers can be attached to Weavers and will disconnect
      automatically when they are deleted.</p>

      \section Job Execution

      <p>In general, jobs are executed in the order they are queued, if they
      have no unresolved dependencies. This behaviour can be used to balance
      I/O, network and CPU load. The SMIV example shows how this can be done.
      </p>

      \section Emitting Signals from Jobs
      <p>To notify the application's GUI of progress or other events, it may
      be desirable to emit signals from the Job objects that can be connected
      to the main thread. Since the job will be executed in another thread,
      such signals are delivered asynchroneously.</p>

      <p>There is a pitfall to this that Job class developers need to be aware
      of. The Job objects are usually created in the main thread, and
      Job::thread() will return the GUI thread's id because of that. When
      signals are emitted, this will fool Qt to consider those to be signals
      between QObjects owned by the same thread, and deliver them
      syncroneously. The solution is to not emit the signals directly, but
      create a helper QObject derived class that has the same signals, create
      an object of this class when the job is executed, and emit the signals
      from there. Since this object will be created in the context of the
      executing thread, the signals will be delivered asyncronously again.</p>

      <p>The Job class in the ThreadWeaver library itself contains such a
      helper class that can be used as a reference for this approach.</p>

      \section Tests Example Code
      <p>Example code is available in the Tests directory. The Construction
      test shows how to use ThreadWeaver in imperative (not event oriented)
      programs. The Jobs test provides a GUI example and displays the
      interaction with the weaver state and it's signals. The Simple
      Multithreaded Image Viewer (SMIV) example shows the use of job
      dependencies.</p>
    */

    Weaver* Weaver::m_instance;

    Weaver::Weaver ( QObject* parent, int inventoryMin, int inventoryMax ) :
        WeaverInterface( parent ) { m_weaverinterface = makeWeaverImpl (
        inventoryMin, inventoryMax ); connect ( m_weaverinterface, SIGNAL (
        finished() ), SIGNAL ( finished() ) ); connect ( m_weaverinterface,
        SIGNAL ( suspended() ), SIGNAL ( suspended() ) ); connect (
        m_weaverinterface, SIGNAL ( jobDone( Job* ) ), SIGNAL ( jobDone ( Job*
        ) ) ); }

    Weaver::~Weaver()
    {
        delete m_weaverinterface;
    }

    WeaverInterface* Weaver::makeWeaverImpl(int inventoryMin, int inventoryMax )
    {
        return new WeaverImpl ( this, inventoryMin,  inventoryMax );
    }

    const State& Weaver::state() const
    {
        return m_weaverinterface->state();
    }

    void Weaver::registerObserver ( WeaverObserver *ext )
    {
        m_weaverinterface->registerObserver ( ext );
    }

    Weaver* Weaver::instance()
    {
	if (m_instance == 0)
	{   // we try to avoid the expensive mutex-lock operation if possible:
            static QMutex mutex;
            QMutexLocker l(&mutex);
            if ( m_instance == 0 )
            {
                m_instance = new Weaver();
            }
        }
	return m_instance;
    }

    void Weaver::enqueue (Job* j)
    {
        m_weaverinterface->enqueue ( j );
    }

    void Weaver::enqueue (const QList<Job*>& jobs)
    {
        m_weaverinterface->enqueue ( jobs );
    }

    bool Weaver::dequeue (Job* j)
    {
        return m_weaverinterface->dequeue ( j );
    }

    void Weaver::dequeue ()
    {
        return m_weaverinterface->dequeue();
    }

    void Weaver::finish ()
    {
        return m_weaverinterface->finish ();
    }

    void Weaver::suspend ()
    {
        return m_weaverinterface->suspend();
    }

    void Weaver::resume ()
    {
        return m_weaverinterface->resume();
    }

    bool Weaver::isEmpty()
    {
        return m_weaverinterface->isEmpty();
    }

    bool Weaver::isIdle()
    {
        return m_weaverinterface->isIdle();
    }

    int Weaver::queueLength()
    {
        return m_weaverinterface->queueLength();
    }

    int Weaver::noOfThreads()
    {
        return m_weaverinterface->noOfThreads();
    }

    void Weaver::requestAbort()
    {
        m_weaverinterface->requestAbort();
    }

}
