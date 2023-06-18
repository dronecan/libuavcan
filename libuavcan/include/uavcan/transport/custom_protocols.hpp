#ifndef UAVCAN_CUSTOM_PROTOCOLS_HPP_INCLUDED
#define UAVCAN_CUSTOM_PROTOCOLS_HPP_INCLUDED

#include <uavcan/driver/can.hpp>
#include <uavcan/util/linked_list.hpp>
#include <uavcan/transport/can_io.hpp>
#include <uavcan/transport/transfer.hpp>

namespace uavcan
{

class CustomTransferListener : public LinkedListNode<CustomTransferListener>
{
public:
	int protocol_;

	CustomTransferListener(const int protocol) :
		protocol_(protocol)
	{;}

	int getCANProtocol() { return protocol_; }

	virtual void handleFrame(const CanRxFrame& can_frame) = 0;
};

// right now this class is useless
class CustomTransferSender
{
public:
	int protocol_;

	CustomTransferSender(const int protocol) :
		protocol_(protocol)
	{;}

	int getCANProtocol() { return protocol_; }
};

};

#endif // UAVCAN_CUSTOM_PROTOCOLS_HPP_INCLUDED
