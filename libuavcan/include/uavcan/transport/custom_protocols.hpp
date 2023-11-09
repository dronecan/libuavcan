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
	Protocol protocol_;

	CustomTransferListener() : protocol_(Protocol::Standard) {;}

	CustomTransferListener(const Protocol protocol) :
		protocol_(protocol)
	{;}

	Protocol getCANProtocol() { return protocol_; }

	virtual bool handleFrame(const CanRxFrame& can_frame, const Protocol protocol) = 0;
};

// right now this class is useless
class CustomTransferSender
{
public:
	Protocol protocol_;

	CustomTransferSender() : protocol_(Protocol::Standard) {;}

	CustomTransferSender(const Protocol protocol) :
		protocol_(protocol)
	{;}

	Protocol getCANProtocol() { return protocol_; }
};

};

#endif // UAVCAN_CUSTOM_PROTOCOLS_HPP_INCLUDED
