// for timing functions
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <sys/time.h>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#include <ns3-dev/ns3/core-module.h>
#include <ns3-dev/ns3/csma-module.h>
#include <ns3-dev/ns3/internet-module.h>
#include <ns3-dev/ns3/network-module.h>
#include <ns3-dev/ns3/point-to-point-module.h>
#include <ns3-dev/ns3/applications-module.h>
#include <ns3-dev/ns3/bridge-helper.h>
#include <ns3-dev/ns3/onoff-application.h>
#include <ns3-dev/ns3/packet-sink.h>
#include <ns3-dev/ns3/simulator.h>
#include <ns3-dev/ns3/ipv4-static-routing-helper.h>
#include <ns3-dev/ns3/ipv4-list-routing-helper.h>
#include <ns3-dev/ns3/ipv4-nix-vector-helper.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-rate-l3-tracer.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-seqs-app-tracer.h>
#include <ns3-dev/ns3/ndnSIM-module.h>

using namespace ns3;
using namespace boost;

namespace br = boost::random;

typedef struct timeval TIMER_TYPE;
#define TIMER_NOW(_t) gettimeofday (&_t,NULL);
#define TIMER_SECONDS(_t) ((double)(_t).tv_sec + (_t).tv_usec*1e-6)
#define TIMER_DIFF(_t1, _t2) (TIMER_SECONDS (_t1)-TIMER_SECONDS (_t2))

NS_LOG_COMPONENT_DEFINE ("TCPBulkTest");

// Number generator
br::mt19937_64 gen;

int obtain_Num(int min, int max) {
    br::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

// Obtains a random list of clients and servers
tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > assignClientsandServers(int num_clients, int num_servers) {

	char buffer[250];

	// Obtain the global list of Nodes in the simulation
	NodeContainer global = NodeContainer::GetGlobal ();

	// Get the number of nodes in the simulation
	uint32_t size = global.GetN ();

	sprintf(buffer, "assignClientsandServers, we have %d nodes", size);

	NS_LOG_INFO (buffer);

	sprintf(buffer, "assignClientsandServers, %d clients, %d servers", num_clients, num_servers);

	// Check that we haven't asked for a scenario where we don't have enough Nodes to fufill
	// the requirements
	if (num_clients + num_servers > size) {
		return tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > ();
	}

	std::vector<Ptr<Node> > globalmutable;

	// Copy the global list into a mutable vector
	for (uint32_t i = 0; i < size; i++) {
		globalmutable.push_back (global.Get(i));
	}

	uint32_t clientMin = size - num_clients - 1;
	uint32_t serverMin = clientMin - num_servers;

	std::vector<Ptr<Node> > ClientContainer;
	std::vector<Ptr<Node> > ServerContainer;

	// Apply Fisher-yates shuffle - start with clients
	for (uint32_t i = size-1; i > clientMin; i--) {
		// Get a random number
		int toSwap = obtain_Num (0, i);
		// Push into the client container the number we got from the global vector
		ClientContainer.push_back (globalmutable[toSwap]);
		// Swap the obtained number with the last element
		std::swap (globalmutable[toSwap], globalmutable[i]);
	}

	// Apply Fischer-yates shuffle - servers
	for (uint32_t i = clientMin; i > serverMin; i--) {
		// Get a random number
		int toSwap = obtain_Num(0, i);
		// Push into the client container the number we got from the global vector
		ServerContainer.push_back(globalmutable[toSwap]);
		// Swap the obtained number with the last element
		std::swap (globalmutable[toSwap], globalmutable[i]);
	}

	return tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > (ClientContainer,ServerContainer);
}


int main (int argc, char *argv[])
{
	// These are our scenario arguments
	uint32_t contentsize = 0; // Size in bytes of the content to transfer
	uint32_t clients = 1; // Number of clients in the network
	uint32_t servers = 1; // Number of servers in the network
	uint32_t networks = 4; // Number of additional nodes in the network

	// Temporary storage for strings
	char buffer[250];

	// Make sure to seed our random
	gen.seed(std::time(0));

	//
	// Allow the user to override any of the defaults at
	// run-time, via command-line arguments
	//
	CommandLine cmd;
	cmd.AddValue ("contentsize",
			"Total number of bytes for application to send", contentsize);
	cmd.AddValue ("clients", "Total number of clients in the network", clients);
	cmd.AddValue ("servers", "Total number of servers in the network", servers);
	cmd.AddValue ("networks", "Number of additional nodes in the network", networks);
	cmd.Parse (argc, argv);

	NS_LOG_INFO ("Create nodes");
	NodeContainer nodes;
	nodes.Create (clients + servers + networks);

	NS_LOG_INFO ("Create channels");
	//PointToPointHelper pointToPoint;
	//pointToPoint.SetChannelAttribute ("Delay", StringValue("2ms"));
	//pointToPoint.SetDeviceAttribute ("DataRate", StringValue("100Mbps"));

	CsmaHelper csma;
	csma.SetChannelAttribute("Delay", StringValue("2ms"));
	csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));

	NetDeviceContainer devices = csma.Install(nodes);

	BridgeHelper bridge;
/*	NetDeviceContainer bridgeDev;
	NetDeviceContainer nodeBridgeDev = NetDeviceContainer ();
	NetDeviceContainer assignableDevs = NetDeviceContainer ();

	// Install the p2p channels
	NetDeviceContainer devices = NetDeviceContainer ();

	for (int i = 0; i < users; i++)
	{
		devices.Add (pointToPoint.Install (nodes.Get (i), nodes.Get (i+1)));
	}

	NS_LOG_INFO ("Preparing to bridge");

	// Find out which interfaces we need to bridge
	for (int i = 0; i < devices.GetN (); i++)
	{
		Ptr<NetDevice> tmp = devices.Get (i);

		uint32_t nodeNum = tmp->GetNode ()->GetId ();

		// We know that Node 0 is our central node, so we save all those
		// interfaces to bridge later
		// The rest we save in another container
		if (nodeNum == 0)
		{
			sprintf (buffer, "Found bridge Device: %d", nodeNum);
			NS_LOG_INFO (buffer);
			nodeBridgeDev.Add(tmp);
		}
		else
		{
			sprintf (buffer, "Found normal Device: %d", nodeNum);
			NS_LOG_INFO (buffer);
			assignableDevs.Add(tmp);
		}
	}

	// If we have more than one device in the nodeBridgeDev, we have to
	// bridge them
	if (nodeBridgeDev.GetN() > 1)
	{
		NS_LOG_INFO ("Bridging!");
		// Bridging
		bridgeDev = bridge.Install (nodes.Get (0), nodeBridgeDev);
		// Save the new device to the IP assignable NetDeviceContainer
		assignableDevs.Add(bridgeDev);
	}
	else
	{
		NS_LOG_INFO ("No need to bridge, continuing");
		// Just merge the devices we found
		assignableDevs.Add(nodeBridgeDev);
	}

	sprintf (buffer, "We have %d assignable devices", assignableDevs.GetN());
	NS_LOG_INFO (buffer);

	// Setup NixVector Routing
	Ipv4NixVectorHelper nixRouting;
	Ipv4StaticRoutingHelper staticRouting;

	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);
	list.Add (nixRouting, 10);*/

	// Install the Internet stack on the nodes
	InternetStackHelper stack;
	stack.Install (nodes);

	// Hardware in place, add IP addresses
	NS_LOG_INFO ("Assign IP Addresses");
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer iC = ipv4.Assign (devices);

	// Calculate routing tables
	//NS_LOG_INFO ("Populating Routing tables");
	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	NS_LOG_INFO ("Obtaining the clients and servers");
	// Obtain the random lists of server and clients
	tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > t = assignClientsandServers(clients, servers);

	// Separate the tuple into clients and servers
	std::vector<Ptr<Node> > clientVector = t.get<0> ();
	std::vector<Ptr<Node> > serverVector = t.get<1> ();

	NodeContainer clientNodes;
	std::vector<uint32_t> clientNodeIds;
	NodeContainer serverNodes;
	std::vector<uint32_t> serverNodeIds;

	// We have to manually introduce the Ptr<Node> to the NodeContainers
	for (uint32_t i = 0; i < clients ; i++)
	{
		Ptr<Node> tmp = clientVector[i];

		uint32_t nodeNum = tmp->GetId();

		sprintf (buffer, "Adding client node: %d", nodeNum);
		NS_LOG_INFO (buffer);

		clientNodes.Add(tmp);
		clientNodeIds.push_back(nodeNum);
	}

	// Do the same for the server NodeContainer
	for (uint32_t i = 0; i < servers; i++)
	{
		Ptr<Node> tmp = serverVector[i];

		uint32_t nodeNum = tmp->GetId();

		sprintf (buffer, "Adding server node: %d", nodeNum);
		NS_LOG_INFO (buffer);

		serverNodes.Add(tmp);
		serverNodeIds.push_back(nodeNum);
	}

	// Port for communication
	uint16_t port = 1027;

	NS_LOG_INFO ("Create bulk application");
	sprintf (buffer, "Bulk application to transfer %d bytes", contentsize);
	NS_LOG_INFO (buffer);

	// Create a BulkSendApplication and install it on the server nodes
	std::vector<BulkSendHelper> sources;

	// Go through the IPv4 devices, single out the servers and install the clients
	for (int i = 0; i < devices.GetN (); i++)
	{
		Ptr<NetDevice> tmp = devices.Get (i);

		uint32_t nodeNum = tmp->GetNode ()->GetId ();

		// Check if the node we obtained is a client or server
		std::vector<uint32_t>::iterator result1 = std::find(serverNodeIds.begin(), serverNodeIds.end(), nodeNum);

		if (result1 == serverNodeIds.end())
		{
			// We have a potential client
			std::vector<uint32_t>::iterator result2 = std::find(clientNodeIds.begin(), clientNodeIds.end(), nodeNum);

			if (result2 != clientNodeIds.end())
			{
				// We have a client node, obtain IP to create the applications
				sprintf (buffer, "Adding Application client to node: %d", nodeNum);
				NS_LOG_INFO (buffer);
				// Make the bulk sender attach to client addresses
				sources.push_back(BulkSendHelper ("ns3::TcpSocketFactory",
						InetSocketAddress (iC.GetAddress (i), port)));

				sources[sources.size()-1].SetAttribute ("MaxBytes", UintegerValue (contentsize));
			}
		}
	}

	sprintf (buffer, "Attaching applications");

	NS_LOG_INFO (buffer);

	// Attach the application to the serverNum node
	ApplicationContainer sourceApps = ApplicationContainer ();

	for (int i = 0; i < sources.size(); i++) {
		sourceApps.Add(sources[i].Install (serverNodes));
	}

	// Begin and stop the bulk sender at the following times
	sourceApps.Start (Seconds (1.0));
	sourceApps.Stop (Seconds (10.0));

	NS_LOG_INFO ("Create bulk clients");

	// Create a PacketSinkApplication and install on all but serverNum
	PacketSinkHelper sink ("ns3::TcpSocketFactory",
			InetSocketAddress (Ipv4Address::GetAny (), port));

	// Install the sink application on all clients
	ApplicationContainer sinkApps = sink.Install (clientNodes);
	sinkApps.Start (Seconds (1.0));
	sinkApps.Stop (Seconds (10.0));

	NS_LOG_INFO ("Installing tracers");

	char filename[250];

	sprintf (filename, "results/disaster-tcp-test-rate-trace-%02d.txt", clients);

	// Install the ndnSIM tracers for IPv4
	tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<Ipv4RateL3Tracer> > > rateTracers = Ipv4RateL3Tracer::InstallAll (filename, Seconds (1.0));

	sprintf (filename, "results/disaster-tcp-test-app-delays-trace-%02d.txt", clients);

	tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<Ipv4SeqsAppTracer> > > seqApps = Ipv4SeqsAppTracer::InstallAll(filename);

	sprintf (filename, "results/disaster-tcp-test-drop-trace-%02d.txt", clients);
	L2RateTracer::InstallAll (filename, Seconds (0.5));

	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop (Seconds (10.0));
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
}
