# Distributed-KV

This project is a fork from [KVell](https://dl.acm.org/doi/abs/10.1145/3341301.3359628) project. I am trying to implement client/server key-value store using KVell implementation. The objective is to measure the networking overhead. Therefore, I found it suitable to measure and compare different networking approach such as TCP/IP and RDMA. For future work, I am interested in using [eRPC](https://www.usenix.org/conference/nsdi19/presentation/kalia) as another approach for implementing Distributed KVell.

## RDMA

Like sockets in traditional socket based programming, queue pair (QP) defines the address of the communication endpoints in RDMA based connectivity. Therefore, Each communication endpoint needs to create a QP in order to talk to each other. There are three different types of QP:

1. Reliable Connection (RC)

2. Unreliable Connection (UC)

3. Unreliable Datagram (UD)

When QP is defined, initialized, and connected using one of the RC or UC types, it can only talk to **one other** QP. Otherwise, if QP is created as UD, it is able to talk to **any other** QPs.   


In RDMA based programming, verb is a term that defines the types of communication operations. There are two different communication primitives: **channel semantics (send/receive)** and **memory semantics (read/write)**. If we only consider how data is delivered to the other end, channel semantics involves both communication endpoints. Memory semantics only involves one side of the communication endpoint: the sender can write the data directly to the receiver's memory region, or the receiver can read from the target's memory region without notifying the target.

Generally speaking, memory semantics has less overhead compared to channel semantics and thus has higher raw performance; On the other hand, channel semantics involves less programming effort.
