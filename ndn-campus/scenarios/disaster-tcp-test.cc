// for timing functions
#include <cstdlib>
#include <sys/time.h>
#include <fstream>
#include <ctime>

#include <ns3-dev/ns3/core-module.h>
#include <ns3-dev/ns3/internet-module.h>
#include <ns3-dev/ns3/network-module.h>
#include <ns3-dev/ns3/point-to-point-module.h>
#include <ns3-dev/ns3/applications-module.h>
#include <ns3-dev/ns3/onoff-application.h>
#include <ns3-dev/ns3/packet-sink.h>
#include <ns3-dev/ns3/simulator.h>
#include <ns3-dev/ns3/ipv4-static-routing-helper.h>
#include <ns3-dev/ns3/ipv4-list-routing-helper.h>
#include <ns3-dev/ns3/ipv4-nix-vector-helper.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-rate-l3-tracer.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-seqs-app-tracer.h>
#include <ns3-dev/ns3/ndnSIM-module.h>

#include <string>

using namespace ns3;

typedef struct timeval TIMER_TYPE;
#define TIMER_NOW(_t) gettimeofday (&_t,NULL);
#define TIMER_SECONDS(_t) ((double)(_t).tv_sec + (_t).tv_usec*1e-6)
#define TIMER_DIFF(_t1, _t2) (TIMER_SECONDS (_t1)-TIMER_SECONDS (_t2))

NS_LOG_COMPONENT_DEFINE ("TCPBulkTest");

int main (int argc, char *argv[])
{
	uint32_t maxBytes = 0;
	int users = 1;
	std::srand( (unsigned)std::time( 0 ));

	//
	// Allow the user to override any of the defaults at
	// run-time, via command-line arguments
	//
	CommandLine cmd;
	cmd.AddValue ("maxBytes",
			"Total number of bytes for application to send", maxBytes);
	cmd.AddValue("users", "Total number of users", users);
	cmd.Parse (argc, argv);

	NS_LOG_INFO ("Create nodes");
	NodeContainer nodes;
	nodes.Create (users + 1);

	NS_LOG_INFO ("Create channels");
	PointToPointHelper pointToPoint;
	pointToPoint.SetChannelAttribute ("Delay", StringValue("2ms"));
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue("100Mbps"));

	// Install the channel
	NetDeviceContainer* devices = new NetDeviceContainer[users];

	for (int i = 0; i < users; i++)
	{
		devices[i] = pointToPoint.Install(nodes[0], nodes[i+1]);
	}

	// Install the Internet stack on the nodes
	InternetStackHelper stack;
	stack.Install (nodes);

	// Setup NixVector Routing
	Ipv4NixVectorHelper nixRouting;
	Ipv4StaticRoutingHelper staticRouting;

	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);
	list.Add (nixRouting, 10);

	stack.SetRoutingHelper (list); // has effect on the next Install ()

	// Hardware in place, add IP addresses
	NS_LOG_INFO ("Assign IP Addresses");
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer* i = new Ipv4InterfaceContainer[users];

	for (int i = 0; i < users; i++)
	{
		ipv4.Assign (devices[i]);
	}

	int randomNum = std::rand() % users + 1;

	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop (Seconds (10.0));
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
}
