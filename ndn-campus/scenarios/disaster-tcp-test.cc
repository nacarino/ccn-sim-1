// for timing functions
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <sys/time.h>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

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

typedef struct timeval TIMER_TYPE;
#define TIMER_NOW(_t) gettimeofday (&_t,NULL);
#define TIMER_SECONDS(_t) ((double)(_t).tv_sec + (_t).tv_usec*1e-6)
#define TIMER_DIFF(_t1, _t2) (TIMER_SECONDS (_t1)-TIMER_SECONDS (_t2))

NS_LOG_COMPONENT_DEFINE ("TCPBulkTest");

int main (int argc, char *argv[])
{
	uint32_t maxBytes = 0;
	int users = 1;
	std::srand ((unsigned)std::time (0));
	char buffer[250];

	//
	// Allow the user to override any of the defaults at
	// run-time, via command-line arguments
	//
	CommandLine cmd;
	cmd.AddValue ("maxBytes",
			"Total number of bytes for application to send", maxBytes);
	cmd.AddValue ("users", "Total number of users", users);
	cmd.Parse (argc, argv);

	NS_LOG_INFO ("Create nodes");
	NodeContainer nodes;
	nodes.Create (users + 1);

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
	NS_LOG_INFO (buffer);*/

	// Setup NixVector Routing
	//Ipv4NixVectorHelper nixRouting;
	//Ipv4StaticRoutingHelper staticRouting;

	//Ipv4ListRoutingHelper list;
	//list.Add (staticRouting, 0);
	//list.Add (nixRouting, 10);

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

	// Obtain the random server
	int serverNum = std::rand() % (users + 1);

	sprintf (buffer, "Random server chosen as: %d", serverNum);

	NS_LOG_INFO (buffer);

	// Port for communication
	uint16_t port = 1027;

	NS_LOG_INFO ("Create bulk application");

	// Create a BulkSendApplication and install it on serverNum

	std::vector<BulkSendHelper> sources;
	std::vector<uint32_t> addedNodes;

	// Group the clients into a container, assign via their devices
	for (int i = 0; i < devices.GetN (); i++)
	{
		Ptr<NetDevice> tmp = devices.Get (i);

		uint32_t nodeNum = tmp->GetNode ()->GetId ();

		// Check that the node we have isn't our server
		if (nodeNum != serverNum)
		{
			bool addTo = false;
			if (addedNodes.size() == 0)
			{
				// We just started, so add whatever we have
				addedNodes.push_back(nodeNum);
				addTo = true;
			}
			else
			{
				// We already have nodes, check if we haven't already added it to the client list
				std::vector<uint32_t>::iterator result1 = std::find(addedNodes.begin(), addedNodes.end(), nodeNum);

				if (result1 != addedNodes.end())
				{
					sprintf (buffer, "Application already added to node: %d, skipping", nodeNum);
					NS_LOG_INFO (buffer);
				}
				else
				{
					addTo = true;
				}
			}

			if (addTo)
			{
				sprintf (buffer, "Adding Application client to node: %d", nodeNum);
				NS_LOG_INFO (buffer);
				// Make the bulk sender attach to client addresses
				sources.push_back(BulkSendHelper ("ns3::TcpSocketFactory",
						InetSocketAddress (iC.GetAddress (i), port)));

				sources[sources.size()-1].SetAttribute ("MaxBytes", UintegerValue (maxBytes));
			}
		}
	}

	sprintf (buffer, "Attaching application to node: %d ", serverNum);

	NS_LOG_INFO (buffer);

	// Attach the application to the serverNum node
	ApplicationContainer sourceApps = ApplicationContainer ();

	for (int i = 0; i < sources.size(); i++) {
		sourceApps.Add(sources[i].Install (nodes.Get (serverNum)));
	}

	// Begin and stop the bulk sender at the following times
	sourceApps.Start (Seconds (1.0));
	sourceApps.Stop (Seconds (10.0));

	NS_LOG_INFO ("Create bulk clients");

	// Create a PacketSinkApplication and install on all but serverNum
	PacketSinkHelper sink ("ns3::TcpSocketFactory",
			InetSocketAddress (Ipv4Address::GetAny (), port));

	// Create a Node container with every node except the serverNum
	NodeContainer clients = NodeContainer();

	// Group the clients into a container
	for (int i = 0; i < (users +1); i++)
	{
		if (i != serverNum)
		{
			sprintf(buffer, "Adding client: %d", i);
			NS_LOG_INFO(buffer);
			clients.Add (nodes.Get(i));
		}
	}

	// Install the sink application on all clients
	ApplicationContainer sinkApps = sink.Install (clients);
	sinkApps.Start (Seconds (1.0));
	sinkApps.Stop (Seconds (10.0));

	NS_LOG_INFO ("Installing tracers");

	char filename[250];

	sprintf (filename, "results/disaster-tcp-test-rate-trace-%02d.txt", users);

	// Install the ndnSIM tracers for IPv4
	boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<Ipv4RateL3Tracer> > > rateTracers = Ipv4RateL3Tracer::InstallAll (filename, Seconds (1.0));

	sprintf (filename, "results/disaster-tcp-test-app-delays-trace-%02d.txt", users);

	boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<Ipv4SeqsAppTracer> > > seqApps = Ipv4SeqsAppTracer::InstallAll(filename);

	sprintf (filename, "results/disaster-tcp-test-drop-trace-%02d.txt", users);
	L2RateTracer::InstallAll (filename, Seconds (0.5));

	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop (Seconds (10.0));
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
}
