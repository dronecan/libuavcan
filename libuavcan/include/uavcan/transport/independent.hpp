#ifndef UAVCAN_INDEPENDENT_INDEPENDENT_HPP_INCLUDED
#define UAVCAN_INDEPENDENT_INDEPENDENT_HPP_INCLUDED

#include <uavcan/driver/can.hpp>
#include <uavcan/util/linked_list.hpp>
#include <uavcan/transport/can_io.hpp>
#include <uavcan/transport/transfer.hpp>

namespace uavcan
{

class IndependentTransferListener : public LinkedListNode<IndependentTransferListener>
{
public:
	TransferProtocol protocol_;

	IndependentTransferListener(const TransferProtocol protocol) :
		protocol_(protocol)
	{;}

	TransferProtocol getCANProtocol() { return protocol_; }

	virtual void handleFrame(const CanRxFrame& can_frame) = 0;
};

// right now this class is useless
class IndependentTransferSender
{
public:
	TransferProtocol protocol_;

	IndependentTransferSender(const TransferProtocol protocol) :
		protocol_(protocol)
	{;}

	TransferProtocol getCANProtocol() { return protocol_; }
};

};

#endif // UAVCAN_INDEPENDENT_INDEPENDENT_HPP_INCLUDED
