// Standard C++ modules
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <vector>

// Random modules
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>

// ns3 modules
#include <ns3-dev/ns3/applications-module.h>
#include <ns3-dev/ns3/bridge-helper.h>
#include <ns3-dev/ns3/csma-module.h>
#include <ns3-dev/ns3/core-module.h>
#include <ns3-dev/ns3/mobility-module.h>
#include <ns3-dev/ns3/network-module.h>
#include <ns3-dev/ns3/point-to-point-module.h>
#include <ns3-dev/ns3/wifi-module.h>

// ndnSIM modules
#include <ns3-dev/ns3/ndnSIM-module.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-rate-l3-tracer.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-seqs-app-tracer.h>

using namespace ns3;
using namespace boost;

namespace br = boost::random;

typedef struct timeval TIMER_TYPE;
#define TIMER_NOW(_t) gettimeofday (&_t,NULL);
#define TIMER_SECONDS(_t) ((double)(_t).tv_sec + (_t).tv_usec*1e-6)
#define TIMER_DIFF(_t1, _t2) (TIMER_SECONDS (_t1)-TIMER_SECONDS (_t2))

char scenario[250] = "CCNWirelessPerfect";

NS_LOG_COMPONENT_DEFINE (scenario);

int main (int argc, char *argv[])
{
	// These are our scenario arguments
	uint32_t contentsize = 10485760;	// Size in bytes of the content to transfer
	uint32_t aps = 6;					// Number of clients in the network
	uint32_t mobile = 1;				// Number of mobile terminals
	uint32_t clients = 1;				// Number of clients in the network
	uint32_t nodes = 12;				// Number of nodes in the network

	char results[250] = "results";
	char buffer[250];

	CommandLine cmd;

	cmd.AddValue ("aps", "Number of APs", aps);
	cmd.AddValue ("mobile", "Number of mobile terminals", mobile);
	cmd.AddValue ("clients", "Number of clients", clients);
	cmd.AddValue ("network", "Number of network nodes", nodes);
	cmd.AddValue ("contentsize",
			"Total number of bytes for application to send", contentsize);
	cmd.AddValue ("results", "Directory to place results", results);
	cmd.Parse (argc,argv);

	// Node definitions for mobile terminals
	NodeContainer mobileTerminalContainer;
	mobileTerminalContainer.Create(1);

	NodeContainer clientNodes;
	clientNodes.Create (4);

	NodeContainer routerNode;
	routerNode.Create (2);

	NodeContainer lanNode;
	lanNode.Create (1);

	NodeContainer allUserNodes;
	allUserNodes.Add (mobileTerminalContainer);
	allUserNodes.Add (clientNodes);

	NodeContainer allRouterNodes;
	allRouterNodes.Add (routerNode);
	allRouterNodes.Add (lanNode);

	NodeContainer csmaNodes;
	csmaNodes.Add (lanNode);
	csmaNodes.Add (clientNodes);

	// Mobility definition for APs
	MobilityHelper mobilityStations;

	mobilityStations.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (30.0),
			"DeltaX", DoubleValue (30.0),
			"DeltaY", DoubleValue (0.0),
			"GridWidth", UintegerValue (2),
			"LayoutType", StringValue ("RowFirst"));
	mobilityStations.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobilityStations.Install (routerNode);

	MobilityHelper mobilityTerminal;

	mobilityTerminal.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (0.0),
			"DeltaX", DoubleValue (30.0),
			"DeltaY", DoubleValue (0.0),
			"GridWidth", UintegerValue (mobileTerminalContainer.GetN ()),
			"LayoutType", StringValue ("RowFirst"));
	mobilityTerminal.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobilityTerminal.Install (mobileTerminalContainer);

	MobilityHelper OtherNode;

	OtherNode.SetPositionAllocator("ns3::GridPositionAllocator",
			"MinX", DoubleValue (15.0),
			"MinY", DoubleValue (60.0),
			"DeltaX", DoubleValue (0.0),
			"DeltaY", DoubleValue (0.0),
			"GridWidth", UintegerValue (1),
			"LayoutType", StringValue ("RowFirst"));
	OtherNode.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	OtherNode.Install (lanNode);

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");
	wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

	YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default ();
	wifiPhyHelper.SetChannel (wifiChannel.Create ());
	wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
	wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

	Ssid ssid = Ssid("test");

	NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default ();
	wifiMacHelper.SetType("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid),
			"ActiveProbing", BooleanValue (true));

	NetDeviceContainer wifiMTNetDevices = wifi.Install (wifiPhyHelper, wifiMacHelper, mobileTerminalContainer);


	wifiMacHelper.SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid),
			"BeaconGeneration", BooleanValue (true),
			"BeaconInterval", TimeValue (Seconds (0.1)));

	NetDeviceContainer wifiAPNetDevices = wifi.Install (wifiPhyHelper, wifiMacHelper, routerNode.Get (0));

	PointToPointHelper p2p_1gb5ms;
	p2p_1gb5ms.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
	p2p_1gb5ms.SetChannelAttribute ("Delay", StringValue ("5ms"));

	CsmaHelper csma;
	csma.SetChannelAttribute("Delay", StringValue("2ms"));
	csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));

	NetDeviceContainer p2pAPClientDevices = p2p_1gb5ms.Install (routerNode.Get (0), routerNode.Get (1));
	NetDeviceContainer p2pAPLANDevice01 = p2p_1gb5ms.Install (routerNode.Get (0), lanNode.Get (0));
	NetDeviceContainer p2pAPLANDevice02 = p2p_1gb5ms.Install (routerNode.Get (1), lanNode.Get (0));

	csma.Install (csmaNodes);

	ndn::StackHelper ndnHelperRouters;
	ndnHelperRouters.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
	ndnHelperRouters.SetContentStore ("ns3::ndn::cs::Freshness::Lru", "MaxSize", "1000");
	ndnHelperRouters.SetDefaultRoutes (true);
	ndnHelperRouters.Install (allRouterNodes);

	ndn::StackHelper ndnHelperUsers;
	ndnHelperUsers.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	ndnHelperUsers.SetContentStore ("ns3::ndn::cs::Nocache");
	ndnHelperUsers.SetDefaultRoutes (true);
	ndnHelperUsers.Install (allUserNodes);

	char routeType[250];

	sprintf(routeType, "%s", "flood");

	NS_LOG_INFO ("Installing Producer Application");
	// Create the producer on the mobile node
	ndn::AppHelper producerHelper ("ns3::ndn::Producer");
	producerHelper.SetPrefix ("/waseda/sato");
	producerHelper.SetAttribute("StopTime", TimeValue (Seconds(24.0)));
	producerHelper.Install (mobileTerminalContainer);

	NS_LOG_INFO ("Installing Consumer Application");
	// Create the consumer on the randomly selected node
	ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
	consumerHelper.SetPrefix ("/waseda/sato");
	consumerHelper.SetAttribute ("Frequency", DoubleValue (10.0));
	consumerHelper.SetAttribute("StartTime", TimeValue (Seconds(1.5)));
	consumerHelper.SetAttribute("StopTime", TimeValue (Seconds(23)));
	consumerHelper.Install (clientNodes.Get (0));

	/*ndn::AppHelper consumer2Helper ("ns3::ndn::ConsumerCbr");
	consumer2Helper.SetPrefix ("/waseda/sato");
	consumer2Helper.SetAttribute ("Frequency", DoubleValue (10.0));
	consumer2Helper.SetAttribute("StartTime", TimeValue (Seconds(5.0)));
	consumer2Helper.SetAttribute("StopTime", TimeValue (Seconds(10.0)));
	consumer2Helper.Install (clientNodes.Get (1));*/

	// Filename
	char filename[250];

	char fileId[250];

	// Create the file identifier
	sprintf(fileId, "%s-%02d-%03d-%03d.txt", routeType, mobile, clients, nodes);

	sprintf (filename, "%s/%s-rate-trace-%s", results, scenario, fileId);
	ndn::L3RateTracer::InstallAll (filename, Seconds (1.0));

	sprintf (filename, "%s/%s-app-delays-%s", results, scenario, fileId);
	ndn::AppDelayTracer::InstallAll (filename);

	Simulator::Stop (Seconds (28.0));
	Simulator::Run ();
	Simulator::Destroy ();

}
