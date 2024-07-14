# Message

## Property Based Message System

There is a message system that is used to communicate between systems. It is a simple system that allows you to send messages to a system and have them delivered to the system. The message system is a property based system, meaning that messages are just a collection of properties that can be read by the system. The system can then use these properties to determine what to do with the message. This is the most basic dependency that a message system can have.

## Event Bus, Struct Based Message System

The event bus is a simple way to communicate between systems without having to know about each other. It is a publish/subscribe system that allows you to send messages to a topic and have them delivered to all subscribers of that topic.
During a certain 'frame' events/messages are stored and when the frame is over, all messages are delivered to the subscribers. After they are delivered all of the occupied memory is reset to its initial state and a new frame can start.
The dependency here is that messages are structs and thus those structs have to be known by the systems that are communicating. You can either duplicate the structs or have a shared header file that contains the structs.