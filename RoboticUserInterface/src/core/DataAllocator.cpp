#include "robotic_user_interface/core/DataAllocator.h"
#include "robotic_user_interface/core/Communicator.h"

DataAllocator::DataAllocator(QObject* parent) 
	: QObject(parent)
{

}

DataAllocator::~DataAllocator() {


}

void DataAllocator::setCommPtr(QPointer<Communicator> comm) {
	comm_ = comm;
}

void DataAllocator::setConfiguration(std::shared_ptr<Configuration> config)
{
	config_ = config;
}

bool DataAllocator::write(CommunicationConfiguration::CommProtocol protocol, const QByteArray& buffer) {
	if (config_->comm.commProtocol != protocol) {
		return false;
	}
  return comm_->write(buffer);
}

bool DataAllocator::read(CommunicationConfiguration::CommProtocol protocol, DataPktBufferTimePtrVec& vec) {
	if (config_->comm.commProtocol != protocol) {
		return false;
	}
	return comm_->read(vec);
}

void DataAllocator::init(){
	//QObject::connect(comm_, &Communicator::readyRead, this, &DataAllocator::readyRead);
}
