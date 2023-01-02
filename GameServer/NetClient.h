#pragma once

#include "NetMessage.h"
#include "ThreadSafeQueue.h"
#include "NetConnection.h"

template <typename T>
class client_interface
{
public:
	client_interface()
	{}

	virtual ~client_interface()
	{
		// If the client is destroyed, always try and disconnect from server
		Disconnect();
	}

public:
	// Connect to server with hostname/ip-address and port
	bool Connect(const std::string& host, const uint16_t port)
	{
		try
		{
			// Resolve hostname/ip-address into tangiable physical address
			asio::ip::tcp::resolver resolver(m_context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			// Create connection
			m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);

			// Tell the connection object to connect to server
			m_connection->ConnectToServer(endpoints);

			// Start Context Thread
			thrContext = std::thread([this]() { m_context.run(); });
		}
		catch (std::exception& e)
		{
			std::cerr << "Client Exception: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	// Disconnect from server
	void Disconnect()
	{
		// If connection exists, and it's connected then...
		if (IsConnected())
		{
			// ...disconnect from server gracefully
			m_connection->Disconnect();
		}

		// Either way, we're also done with the asio context...				
		m_context.stop();
		// ...and its thread
		if (thrContext.joinable())
			thrContext.join();

		// Destroy the connection object
		m_connection.release();
	}

	// Check if client is actually connected to a server
	bool IsConnected()
	{
		if (m_connection)
			return m_connection->IsConnected();
		else
			return false;
	}

public:
	// Send message to server
	void Send(const message<T>& msg)
	{
		if (IsConnected())
			m_connection->Send(msg);
	}

	// Retrieve queue of messages from server
	ThreadSafeQueue<owned_message<T>>& Incoming()
	{
		return m_qMessagesIn;
	}

protected:
	// asio context handles the data transfer...
	asio::io_context m_context;
	// ...but needs a thread of its own to execute its work commands
	std::thread thrContext;
	// The client has a single instance of a "connection" object, which handles data transfer
	std::unique_ptr<connection<T>> m_connection;

private:
	// This is the thread safe queue of incoming messages from server
	ThreadSafeQueue<owned_message<T>> m_qMessagesIn;
};
