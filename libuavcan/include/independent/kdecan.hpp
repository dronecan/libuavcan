#ifndef UAVCAN_INDEPENDENT_KDECAN_HPP_INCLUDED
#define UAVCAN_INDEPENDENT_KDECAN_HPP_INCLUDED

#include <uavcan/transport/can_io.hpp>
#include <uavcan/driver/can.hpp>
#include <uavcan/transport/transfer.hpp>
#include <uavcan/transport/dispatcher.hpp>
#include <uavcan/transport/can_io.hpp>
#include <uavcan/util/linked_list.hpp>
#include <uavcan/util/lazy_constructor.hpp>

namespace kdecan
{

static const int escNodeIdBroadcast = 1;
static const int escNodeIdOffset = 2;

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

class KdeCanTransferListener : public uavcan::LinkedListNode<KdeCanTransferListener>
{
public:
	kdeCanObjAddr target_object_address_;

	KdeCanTransferListener(const kdeCanObjAddr target_object_address) :
		target_object_address_(target_object_address)
	{;}

	virtual void handleIncomingTransfer(const uint32_t id, const uint8_t dlc, const uint8_t* data) = 0;

	void handleFrame(const uavcan::CanRxFrame& can_frame)
	{
		uint32_t object_address = can_frame.id & 0x000000FF;
		
		if (object_address == target_object_address_)
		{
			handleIncomingTransfer(can_frame.id, can_frame.dlc, can_frame.data);
		}
	}
};

template <kdeCanObjAddr DataType_,
#if UAVCAN_CPP_VERSION >= UAVCAN_CPP11
	typename Callback_ = std::function<void (const uint32_t id, const uint8_t dlc, const uint8_t* data)>
#else
	typename Callback_ = void (*)(const uint32_t id, const uint8_t dlc, const uint8_t* data)
#endif
>
class KdeCanSubscriber
{
public:
	typedef KdeCanSubscriber<DataType_, Callback_> SelfType;
	typedef Callback_ Callback;

	class kdeTransferForwarder : public KdeCanTransferListener
	{
	public:
		SelfType& obj_;

		void handleIncomingTransfer(const uint32_t id, const uint8_t dlc, const uint8_t* data) override
		{
			obj_.handleIncomingTransfer(id, dlc, data);
		}

		kdeTransferForwarder(SelfType& obj, const kdeCanObjAddr target_object_address) :
			KdeCanTransferListener(target_object_address),
			obj_(obj)
		{;}
	};

	Callback callback_;
	kdeTransferForwarder forwarder_;

	KdeCanSubscriber() :
		callback_(),
		forwarder_(*this, DataType_)
	{;}

	void handleIncomingTransfer(const uint32_t id, const uint8_t dlc, const uint8_t* data)
	{
		if (callback_)
		{
			callback_(id, dlc, data);
		}
	}

	void setCallback(Callback callback)
	{
		callback_ = callback;
	}

	KdeCanTransferListener* getKdeListener()
	{
		return &(this->forwarder_);
	}
};

class KdeCanTransferSender
{
public:
	KdeCanTransferSender() {;}

	bool publish(uavcan::CanFrame& can_frame,
		     const uint8_t source_address,
		     const uint8_t destination_address,
		     const kdeCanObjAddr object_address, const uint8_t* data)
	{
		KdeFrame kde_frame(source_address, destination_address, object_address, data);

		return kde_frame.compile(can_frame);
	}
};

};

#endif // UAVCAN_INDEPENDENT_KDECAN_HPP_INCLUDED
