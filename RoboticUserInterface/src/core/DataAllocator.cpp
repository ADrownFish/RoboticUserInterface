#include "robotic_user_interface/core/DataAllocator.h"

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

void DataAllocator::write(CommunicationConfiguration::CommProtocol protocol, const QByteArray& buffer) {
	if (config_->comm.commProtocol != protocol) {
		return;
	}
    comm_->write(buffer);
}

void DataAllocator::read(CommunicationConfiguration::CommProtocol protocol, QByteArray& buffer) {
	if (config_->comm.commProtocol != protocol) {
		return;
	}
	comm_->read(buffer);
}

void DataAllocator::init(){

	QObject::connect(comm_, &Communicator::readyRead, [this]() {
		this->readyRead();
	});
}
