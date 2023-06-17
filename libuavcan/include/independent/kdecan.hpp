#ifndef UAVCAN_INDEPENDENT_KDECAN_HPP_INCLUDED
#define UAVCAN_INDEPENDENT_KDECAN_HPP_INCLUDED

#include <uavcan/transport/independent.hpp>
#include <uavcan/node/abstract_node.hpp>

namespace kdecan
{

static const int escNodeIdBroadcast = 1;
static const int escNodeIdOffset = 2;
static const int minPwmValue = 1100;
static const int maxPwmValue = 1940;

static const int MASTER_NODE_ID = 0;

enum kdeCanObjAddr
{
	ESCInformation = 0,
	PWMThrottle = 1,
	ESCInputThrottle = 6,
	ESCOutputThrottle = 7,
	ESCStatus = 11,
	GetMCUId = 8,
	UpdateNodeAddress = 9,
	StartESCEnumeration = 10,
	Shutdown = 32,
	Restart = 33
};

class KdeFrame
{
public:
	enum { PayloadCapacity = 8 };

	uint8_t source_address_;
	uint8_t destination_address_;
	kdeCanObjAddr object_address_;
	uint8_t data_[PayloadCapacity];

	KdeFrame(const uint8_t source_address,
		 const uint8_t destination_address,
		 kdeCanObjAddr object_address,
		 const uint8_t* data) :
		source_address_(source_address),
		destination_address_(destination_address),
		object_address_(object_address)
	{
		memcpy(data_, data, PayloadCapacity);
	}

	bool compile(uavcan::CanFrame& out_can_frame)
	{
		out_can_frame.id = ((uint32_t)0x00000000) |
		                   ((uint32_t)object_address_) |
		                   (((uint32_t)destination_address_) << 8) |
		                   (((uint32_t)source_address_) << 16) |
		                   uavcan::CanFrame::FlagEFF;

		switch(object_address_)
		{
			case ESCInformation: { out_can_frame.dlc = 0; } break;
			case PWMThrottle: { out_can_frame.dlc = 2; } break;
			case ESCInputThrottle: { out_can_frame.dlc = 0; } break;
			case ESCOutputThrottle: { out_can_frame.dlc = 0; } break;
			case ESCStatus: { out_can_frame.dlc = 0; } break;
			case GetMCUId: { out_can_frame.dlc = 0; } break;
			case UpdateNodeAddress: { out_can_frame.dlc = 8; } break;
			case StartESCEnumeration: { out_can_frame.dlc = 2; } break;
			case Shutdown: { out_can_frame.dlc = 0; } break;
			case Restart: { out_can_frame.dlc = 0; } break;

			default: { return false; }
		}

		// this function will take care of little/big endian conversions
		(void)uavcan::copy(data_, data_ + out_can_frame.dlc, out_can_frame.data);

		return true;
	}
};

template <kdeCanObjAddr DataType_,
#if UAVCAN_CPP_VERSION >= UAVCAN_CPP11
	typename Callback_ = std::function<void (const uint32_t id, const uint8_t dlc, const uint8_t* data)>
#else
	typename Callback_ = void (*)(const uint32_t id, const uint8_t dlc, const uint8_t* data)
#endif
>
class Subscriber
{
public:
	typedef Subscriber<DataType_, Callback_> SelfType;
	typedef Callback_ Callback;

	class kdeTransferForwarder : public uavcan::IndependentTransferListener
	{
	public:
		SelfType& obj_;

		void handleFrame(const uavcan::CanRxFrame& can_frame) override
		{
			obj_.handleIncomingTransfer(can_frame);
		}

		kdeTransferForwarder(SelfType& obj) :
			uavcan::IndependentTransferListener(uavcan::TransferProtocol::KDECANProtocol),
			obj_(obj)
		{;}
	};

	uavcan::INode& node_;
	Callback callback_;
	kdeTransferForwarder forwarder_;
	kdeCanObjAddr target_object_address_;

	Subscriber(uavcan::INode& node) :
		node_(node),
		callback_(),
		forwarder_(*this),
		target_object_address_(DataType_)
	{;}

	void handleIncomingTransfer(const uavcan::CanRxFrame& can_frame)
	{
		uint32_t object_address = can_frame.id & 0x000000FF;
		uint32_t destination_id = (can_frame.id & 0x0000FF00) >> 8;

		if (destination_id == MASTER_NODE_ID && object_address == target_object_address_ && callback_)
		{
			callback_(can_frame.id, can_frame.dlc, can_frame.data);
		}
	}

	void setCallback(Callback callback)
	{
		callback_ = callback;
	}

	uavcan::IndependentTransferListener* getKdeListener()
	{
		return &(this->forwarder_);
	}
};

template <kdeCanObjAddr DataType_>
class Publisher
{
public:
	uavcan::INode& node_;
	kdeCanObjAddr target_object_address_;

	Publisher(uavcan::INode& node) :
		node_(node),
		target_object_address_(DataType_)
	{;}

	bool publish(const uint8_t destination_address,
		     const uint8_t* data)
	{
		KdeFrame kde_frame(MASTER_NODE_ID, destination_address, target_object_address_, data);

		uavcan::CanFrame can_frame;
		kde_frame.compile(can_frame);

		node_.getDispatcher().sendRaw(can_frame,
			              uavcan::TransferProtocol::KDECANProtocol,
			              node_.getMonotonicTime() + uavcan::MonotonicDuration::fromMSec(100),
			              uavcan::MonotonicTime(),
			              uavcan::CanTxQueue::Qos::Volatile,
			              uavcan::CanIOFlags(0),
			              (uint8_t)0xFF);

		// so far we do no checks in this
		return true;
	}
};

};

#endif // UAVCAN_INDEPENDENT_KDECAN_HPP_INCLUDED
