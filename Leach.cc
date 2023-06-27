#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/animation-interface.h"
#include "ns3/attribute-helper.h"
#include "ns3/node.h"
#include "ns3/energy-module.h"
#include "ns3/rv-battery-model.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/lr-wpan-helper.h"
#include "ns3/lr-wpan-net-device.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/spectrum-channel.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/header.h"



#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <math.h> 
#include <random>
#include <cstdarg>
#include <map>
#include <sstream>
#include <numeric>

//#define CLUSTER_PERCENTAGE 0.05
//#define NUM_NODES 100
#define duration 500000
#define MIN 5
#define MAX 50
#define SEUIL 5

using namespace ns3;
using namespace std;

int NUM_NODES = 100;
double CLUSTER_PERCENTAGE = 0.05;

AnimationInterface * anim = 0;
std::map<Ptr<Node>, bool> nodeIsDead;
std::map<Mac16Address , Ptr<Node>> network;

//Fonction pour supprimer les doublons
void removeDuplicates(std::vector<Mac16Address>& vec) 
{
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}


enum MessageType
{
	Msg_Type_ADVERTISE=1,
	Msg_Type_REGISTER=2,
  	Msg_Type_CLUSTERCREATE=3
};


class TypeHeader : public Header
{
	public:
		TypeHeader (MessageType type);

		TypeHeader ();

		static TypeId GetTypeId ();

		TypeId GetInstanceTypeId () const;

		uint32_t GetSerializedSize () const;

		void Serialize (Buffer::Iterator start) const;

		uint32_t Deserialize (Buffer::Iterator start);

		void Print (std::ostream &os) const;

		bool operator ==(const TypeHeader& o) const;

		MessageType getType() const 
		{
			return Type;
		}

		void setType(MessageType type) 
		{
			Type = type;
		}

		bool isValidMessage() const 
		{
			return ValidMessage;
		}

		void setValidMessage(bool validMessage)
		{
			ValidMessage = validMessage;
		}

	public:
		bool ValidMessage;
		MessageType Type;
		u_int8_t NodeType;
		u_int8_t NodeID;
};

//pour pouvoir print la classe comme si c'était une variable
std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t)
{
	setType(t);
	setValidMessage(true);
}


TypeHeader::TypeHeader ()
{
	setValidMessage(true);
}

TypeId	TypeHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("TypeHeader")
			    .SetParent<Header> ()
			    .AddConstructor<TypeHeader> ();
	return tid;
}

TypeId	TypeHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t TypeHeader::GetSerializedSize () const
{
	return 12;
}

void TypeHeader::Serialize (Buffer::Iterator i) const
{
	i.WriteU8 (Type);
	i.WriteU8 (NodeType);
	i.WriteU8 (NodeID);
}

uint32_t	TypeHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	uint8_t type = i.ReadU8 ();
	NodeType = i.ReadU8 ();
	NodeID = i.ReadU8 ();
	ValidMessage = true;

	switch (type)
	{
		case Msg_Type_ADVERTISE:
		case Msg_Type_REGISTER:
        case Msg_Type_CLUSTERCREATE:
			{
				Type = (MessageType) type;
				break;
			}
		default:
			{
				ValidMessage = false;
				break;
			}
	}

	uint32_t dist = i.GetDistanceFrom (start);
	//NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void TypeHeader::Print (std::ostream &os) const
{	
	switch (Type)
	{
		case Msg_Type_ADVERTISE:
		{
			os << "ADVERTISE";
			break;
		}
		case Msg_Type_REGISTER:
		{
			os << "REGISTER";
			break;
		}
       case   Msg_Type_CLUSTERCREATE :
		{
			os << "CLUSTERCREATE";
		}
		default:
		{
			os << "UNKNOWN_TYPE";
			break;
		}
	}
}

bool TypeHeader::operator== (TypeHeader const & o) const
{
	return (Type == o.Type && ValidMessage == o.ValidMessage);
}

std::ostream & operator<< (std::ostream & os, TypeHeader const & h)
{
	h.Print (os);
	return os;
}


//============== définition de la calsse ClusterAdvertiseHeader ====================

//==================================================================================


class ClusterAdvertiseHeader : public Header
{
	public:
		ClusterAdvertiseHeader();

		~ClusterAdvertiseHeader();

		static TypeId GetTypeId ();

		TypeId GetInstanceTypeId () const;

		uint32_t GetSerializedSize () const;

		void Serialize (Buffer::Iterator i) const;

		uint32_t Deserialize (Buffer::Iterator start);

		void Print (std::ostream &os) const;

		bool operator== (ClusterAdvertiseHeader const & o) const;

	private:
		
};


std::ostream & operator<< (std::ostream & os, ClusterAdvertiseHeader const & h);

NS_OBJECT_ENSURE_REGISTERED (ClusterAdvertiseHeader);

//========= définition des fonctions de la calsse ClusterAdvertiseHeader ===========

//==================================================================================

ClusterAdvertiseHeader::ClusterAdvertiseHeader ()
{

}

ClusterAdvertiseHeader::~ClusterAdvertiseHeader ()
{

}

TypeId ClusterAdvertiseHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ClusterAdvertiseHeader")
			.SetParent<Header> ()
			.AddConstructor<ClusterAdvertiseHeader> ();
	return tid;
}

TypeId ClusterAdvertiseHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t ClusterAdvertiseHeader::GetSerializedSize () const
{
	return 0;
}

void ClusterAdvertiseHeader::Serialize (Buffer::Iterator i) const
{
	/*
	for (int j =0; j < 10 ; ++j)
	{
		i.WriteU8 (Object[j]);
	}
	i.WriteU32(RegisterDuration);
	*/
}

uint32_t ClusterAdvertiseHeader::Deserialize (Buffer::Iterator start)
{
	/*
	Buffer::Iterator i = start;
	for (int j =0; j <10; ++j)
	{
		Object[j] = i.ReadU8 ();
	}
	
	RegisterDuration = i.ReadU32();
	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	*/
	return 0;
}

void ClusterAdvertiseHeader::Print (std::ostream & os) const
{
	/*
	os << "objet de message: " << Object  ;
	*/
}

std::ostream &operator<< (std::ostream & os, ClusterAdvertiseHeader const & h)
{
	h.Print (os);
	return os;
}

bool ClusterAdvertiseHeader::operator== (ClusterAdvertiseHeader const & o) const
{
	/*
	return (Object[10]  == o.Object[10] );
	*/
	return 1;
}


//=============== définition de la calsse ClusterRegisterHeader ====================

//==================================================================================

class ClusterRegisterHeader : public Header
{
	public:
		ClusterRegisterHeader();

		~ClusterRegisterHeader();

		static TypeId GetTypeId ();
		
		TypeId GetInstanceTypeId () const;
		
		uint32_t GetSerializedSize () const;
		
		void Serialize (Buffer::Iterator i) const;
		
		uint32_t Deserialize (Buffer::Iterator start);
		
		void Print (std::ostream &os) const;
		
		bool operator== (ClusterRegisterHeader const & o) const;

	public:
		//u_int8_t LastRoundSinceCH;
		//double Energy;
};

std::ostream & operator<< (std::ostream & os, ClusterRegisterHeader const & h);

NS_OBJECT_ENSURE_REGISTERED (ClusterRegisterHeader);

//========== définition des fonctions de la calsse ClusterRegisterHeader ===========

//==================================================================================

ClusterRegisterHeader::ClusterRegisterHeader ()
{
	
}

ClusterRegisterHeader::~ClusterRegisterHeader ()
{

}

TypeId ClusterRegisterHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ClusterRegisterHeader")
						.SetParent<Header> ()
						.AddConstructor<ClusterRegisterHeader> ();
	return tid;
}

TypeId ClusterRegisterHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t ClusterRegisterHeader::GetSerializedSize () const
{
	return 0;
}

void ClusterRegisterHeader::Serialize (Buffer::Iterator i) const
{
	//i.WriteU8 (LastRoundSinceCH);
	//i.WriteU8 (Energy);
}

uint32_t ClusterRegisterHeader::Deserialize (Buffer::Iterator start)
{
	/*
	Buffer::Iterator i = start;
	
	LastRoundSinceCH = i.ReadU8 ();
	Energy = i.ReadU8 ();

	uint32_t dist = i.GetDistanceFrom (start);
	//NS_ASSERT (dist == GetSerializedSize ());
	return dist;
	*/
	return 0;
}

void ClusterRegisterHeader::Print (std::ostream &os) const
{
	//os << "BidderRegisterDetail " << BidderRegisterDetail;
}

std::ostream &operator<< (std::ostream & os, ClusterRegisterHeader const & h)
{
	h.Print (os);
	return os;
}

bool ClusterRegisterHeader::operator== (ClusterRegisterHeader const & o) const
{
	//return (BidderRegisterDetail[5] == o.BidderRegisterDetail[5] );
	return 1;
}

//============== définition de la calsse ClusterCreateHeader =======================

//==================================================================================

class ClusterCreateHeader : public Header
{
	public:
		ClusterCreateHeader();

		~ClusterCreateHeader();
		
		static TypeId GetTypeId ();
		
		TypeId GetInstanceTypeId () const;
		
		uint32_t GetSerializedSize () const;
		
		void Serialize (Buffer::Iterator i) const;
		
		uint32_t Deserialize (Buffer::Iterator start);
		
		void Print (std::ostream &os) const;
		
		bool operator== (ClusterCreateHeader const & o) const;

	public:
		//vector de Mac16Address des membres
		vector<Mac16Address> ClusterMembers;
};


std::ostream & operator<< (std::ostream & os, ClusterCreateHeader const & h);

NS_OBJECT_ENSURE_REGISTERED (ClusterCreateHeader);

//===========définition des fonctions de la calsse ClusterCreateHeader==============

//==================================================================================

ClusterCreateHeader::ClusterCreateHeader ()
{
	
}

ClusterCreateHeader::~ClusterCreateHeader ()
{
	
}

TypeId ClusterCreateHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ClusterCreateHeader")
					.SetParent<Header> ()
					.AddConstructor<ClusterCreateHeader> ();
	return tid;
}

TypeId ClusterCreateHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t ClusterCreateHeader::GetSerializedSize () const
{
	return ClusterMembers.size() * 4;
}

void ClusterCreateHeader::Serialize (Buffer::Iterator i) const
{
	
	//i.WriteU32 (ClusterMembers.size());
	Mac16Address ad;
		for (uint j = 0 ; j < ClusterMembers.size() ; j++)
		{
			ad = ClusterMembers[j];
			WriteTo(i,ad);
		}
	
}

uint32_t ClusterCreateHeader::Deserialize (Buffer::Iterator start)
{
	
	Buffer::Iterator i = start;
	Mac16Address ad;

	for (uint j = 0 ; j < ClusterMembers.size() ; ++j)
	{
		ReadFrom(i,ad);
		ClusterMembers[j] = ad;
	}

	uint32_t dist = i.GetDistanceFrom (start);
	//NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void ClusterCreateHeader::Print (std::ostream &os) const
{
	//os << " provider list " << ACL;
}

std::ostream &operator<< (std::ostream & os, ClusterCreateHeader const & h)
{
	h.Print (os);
	return os;
}

bool ClusterCreateHeader::operator== (ClusterCreateHeader const & o) const
{
	return (ClusterMembers.size() == o.ClusterMembers.size() );
}

//=============== définition des fonctions de la calsse Leach =====================

//============================ Application ========================================

// Dans l'application pour tous les noeuds faire generer un nombre aleatoire entre 0 et 1 
// Ensuite le comparer au threshold  

class Leach : public Application 
{
	public:
		Leach();
		
		virtual ~Leach();
		
		static TypeId GetTypeId ();

		void Generate_Random_Number ();

		void Threshold ();

		bool isCH();
		
		void SendClusterAdvertise ();
		
		void SendClusterRegister ();

		void SendClusterCreate ();
		
		void DataIndication(McpsDataIndicationParams params, Ptr<Packet> packet);

		void ReceiveClusterAdvertise (McpsDataIndicationParams params , Ptr<Packet> packet);

		void ReceiveClusterRegister (McpsDataIndicationParams params , Ptr<Packet> packet);
		
		void ReceiveClusterCreate (McpsDataIndicationParams params , Ptr<Packet> packet);
		
		void Choose_CH();

		void RemEnergy ();

		void Round();

		void DeadNode ();
		
		virtual void StartApplication ();

		void CH_Characteristics ();

		void ResetApplication ();

		static void StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState);

		//void SetAttribute (std::string name, const AttributeValue &value);
		
		virtual void StopApplication ();
		
		virtual void DoDispose ();

	public:

		EventId m_event;
		EventId e_event;
		
		u_int8_t NodeType;
		Mac16Address Node_Mac;					
		Mac16Address CH_Adress;	
		Mac16Address NCH_Adress;

		std::map<Mac16Address , int> CH_Propositions;			
	
		int ClusterID ;	 				
		vector<Mac16Address> ClusterMembers;

		MessageType Type;				
	
		int RoundSinceCH;
		int ActualRound;
		bool Not_CH_Yet;

		double r_randomnumber;
		double Tr;
};

NS_OBJECT_ENSURE_REGISTERED (Leach);

Leach::Leach()
{
	ActualRound = 0;
	RoundSinceCH = 0;
	Not_CH_Yet = true;
}

Leach::~Leach()
{

}

TypeId Leach::GetTypeId()
{
	static TypeId tid = TypeId ("ns3::Leach")
			.SetParent<Application> ()
			.SetGroupName("Applications")
			.AddConstructor<Leach> ();
			/*
			.AddAttribute ("NodeType", "The Type of the node , CH or NCH.",
					UintegerValue (0),
					MakeUintegerAccessor (&Leach::NodeType),
					MakeUintegerChecker<uint32_t> ());
			*/

	return tid;
}


//Génerer un nombre aléatoire pour le comparer au threshold 
void Leach::Generate_Random_Number ()
{
	//affectation d'un random number entre 0 et 1
	r_randomnumber = (double)rand()/RAND_MAX;
}

//Calculer le threshold pour le round r
void Leach::Threshold ()
{
	//La formule du threshold de Leach
	Tr = CLUSTER_PERCENTAGE/(double)(1-CLUSTER_PERCENTAGE*(ActualRound%(int)(1/CLUSTER_PERCENTAGE)));
}

//Vérifier si le noeud est cluster head au round r
bool Leach::isCH ()
{
	if (Not_CH_Yet == true || RoundSinceCH >= (int)1/CLUSTER_PERCENTAGE)
	{
		if(r_randomnumber < Tr)
		{
			Not_CH_Yet = false; 
			NodeType = 1; // 1 pour CH
			RoundSinceCH = ActualRound - RoundSinceCH;
			return true;
		}
	}
	NodeType = 0; // 0 pour NCH
	return false;
}

void Leach::SendClusterAdvertise ()
{
	//initialiser un header 
	ClusterAdvertiseHeader AD_Header;

	Ptr<Packet> packet = Create<Packet> (100);
	packet->AddHeader (AD_Header);
	//initialiser un autre header
	TypeHeader tHeader (Msg_Type_ADVERTISE);
	//changer les parametres du header
	tHeader.NodeType = 1;
	tHeader.NodeID = GetNode()->GetId();

	//l'ajouter
	packet->AddHeader (tHeader);
	Mac16Address destination("ff:ff");

	// Créer les paramètres pour l'envoi du paquet
	McpsDataRequestParams params;
	params.m_srcAddrMode = SHORT_ADDR ;
	params.m_dstAddrMode = SHORT_ADDR ;
	//params.m_dstPanId = 0;
	params.m_dstAddr = destination;
	params.m_msduHandle = 0;
  	params.m_txOptions = TX_OPTION_NONE ;


  	//Récupérer le LrWpan NetDevice du node
  	//Peut être vérifier que le netdevice est bien un LrWpan NetDevice?
  	Ptr<LrWpanNetDevice> Net_Device = DynamicCast<LrWpanNetDevice>(GetNode()->GetDevice(0));

	// Envoyer le paquet
	Net_Device->GetMac()->McpsDataRequest(params, packet);
}

void Leach::SendClusterRegister ()
{
	//Initialiser les deux header 
	ClusterRegisterHeader ClusterRegister;

	//ClusterRegister.LastRoundSinceCH = RoundSinceCH;
	//ClusterRegister.Energy = GetNode()->GetObject<BasicEnergySource>()->GetRemainingEnergy();


	TypeHeader tHeader(Msg_Type_REGISTER);
	//changer les parametres du header
	tHeader.NodeType = 0;
	tHeader.NodeID = GetNode()->GetId();

	//Créer le packet
	Ptr<Packet> packet = Create<Packet> (100);

	//Ajouter les header au packet
	packet->AddHeader (ClusterRegister);
	packet->AddHeader (tHeader);

	// Créer les paramètres pour l'envoi du paquet
	McpsDataRequestParams params;
	params.m_srcAddrMode = SHORT_ADDR ;
	params.m_dstAddrMode = SHORT_ADDR ;
	params.m_dstPanId    = 0;
	params.m_dstAddr     = CH_Adress ;
	params.m_msduHandle  = 0;
	params.m_txOptions   = TX_OPTION_NONE ;

	//Récupérer le LrWpan NetDevice du node
  	//Peut être vérifier que le netdevice est bien un LrWpan NetDevice?
  	Ptr<LrWpanNetDevice> Net_Device = DynamicCast<LrWpanNetDevice>(GetNode()->GetDevice(0));

	Net_Device->GetMac()->McpsDataRequest(params, packet);
}

void Leach::SendClusterCreate ()
{
	if (NodeType == 1)
	{
		ClusterCreateHeader ClusterCreate;

 		stringstream msgx;

	    msgx <<"Membres du Cluster" ;

        uint16_t packetSize = 500;

        Ptr<Packet> packet = Create<Packet>((uint8_t*) msgx.str().c_str(), packetSize);

        cout << ClusterMembers.size() <<endl;
		removeDuplicates(ClusterMembers);

		ClusterCreate.ClusterMembers = ClusterMembers;

		TypeHeader tHeader (Msg_Type_CLUSTERCREATE);
        tHeader.NodeType = 0;
		tHeader.NodeID = GetNode()->GetId();
        tHeader.ValidMessage = true;
        
 		packet->AddHeader (ClusterCreate);
	    packet->AddHeader (tHeader);

	    //Récupérer le LrWpan NetDevice du node
  		//Peut être vérifier que le netdevice est bien un LrWpan NetDevice?
  		Ptr<LrWpanNetDevice> Net_Device = DynamicCast<LrWpanNetDevice>(GetNode()->GetDevice(0));

  		ofstream myfile ;
	    myfile.open ("Clusters.txt", std::ofstream::out | std::ofstream::app);
       	if (!myfile.is_open()) 
       	{
    	   	std::cout << "Impossible d'ouvrir le fichier." << std::endl;
       	}
       	//ajouter les Addresses mac des membres du cluster dans le file
       	myfile << "CH " << Net_Device->GetMac()->GetShortAddress() << " " << GetNode()->GetObject<BasicEnergySource>()->GetRemainingEnergy() << " " << ActualRound<<"\n";
       	for(uint i = 0 ; i < ClusterMembers.size() ; i++)
       	{ 
       		myfile << ClusterMembers[i] <<" "<< network[ClusterMembers[i]]->GetApplication(0)->GetObject<Leach>()->RoundSinceCH <<" "<< network[ClusterMembers[i]]->GetObject<BasicEnergySource>()->GetRemainingEnergy() <<";";
       	}
       	myfile <<"\n";
		myfile.close(); 

	    //soit faire une boucle for pour envoyer a tous les membres
	    //soit faire la boucle for dehors et mettre a chaque fois 
	    //l'addresse du noeud i en parametre 
  		// Créer les paramètres pour l'envoi du paquet
		McpsDataRequestParams params;
		params.m_srcAddrMode = SHORT_ADDR ;
		params.m_dstAddrMode = SHORT_ADDR ;
		//params.m_dstPanId    = 0;
		params.m_msduHandle  = 0;
		params.m_txOptions   = TX_OPTION_NONE ;

		for (uint i =0 ; i < ClusterMembers.size() ; i++)
		{
			params.m_dstAddr = ClusterMembers[i];
			Net_Device->GetMac()->McpsDataRequest(params, packet);
		}
	}
}

void Leach::DataIndication(McpsDataIndicationParams params, Ptr<Packet> packet)
{
	TypeHeader tHeader ;
	packet->RemoveHeader (tHeader);

	Type = tHeader.getType();
	switch (Type)
	{
		case Msg_Type_ADVERTISE:
		{
			ReceiveClusterAdvertise (params, packet);
			break;
		}
		case Msg_Type_REGISTER:
		{
			ReceiveClusterRegister (params, packet);
			break;
		}
		case Msg_Type_CLUSTERCREATE:
		{
			ReceiveClusterCreate (params, packet);
			break;
		}
	}
}

void Leach::Choose_CH()
{
	if (!(CH_Propositions.empty()))
	{
		auto mac_max = CH_Propositions.begin();
		int max = mac_max->second ;
		for(auto it = CH_Propositions.begin() ; it != CH_Propositions.end() ; ++it)
		{
			if (max < it->second )
			{
				max = it->second;
				mac_max = it;
			}
		}
		CH_Adress = mac_max->first;
	}
}

void Leach::ReceiveClusterAdvertise(McpsDataIndicationParams params , Ptr<Packet> packet)
{

	ClusterAdvertiseHeader AdvHeader;
	packet->RemoveHeader (AdvHeader);

	if (NodeType == 0)
	{
		CH_Propositions.insert(std::pair<Mac16Address , int>(params.m_srcAddr
			, static_cast<int>(params.m_mpduLinkQuality)));
		Choose_CH();
		SendClusterRegister();
	}
}

void Leach::ReceiveClusterRegister (McpsDataIndicationParams params , Ptr<Packet> packet)
{
	ClusterRegisterHeader ClusterRegister;
	packet->RemoveHeader(ClusterRegister);


	Ptr<LrWpanNetDevice> Net_Device = DynamicCast<LrWpanNetDevice>(GetNode()->GetDevice(0));

	if (NodeType == 1 )
	{
		if (params.m_dstAddr == DynamicCast<LrWpanNetDevice>(GetNode()->GetDevice(0))->GetMac()->GetShortAddress())
		{
			ofstream myfile ;
		    myfile.open ("Packages.txt", std::ofstream::out | std::ofstream::app);
	       	if (!myfile.is_open()) 
	       	{
	    	   	std::cout << "Impossible d'ouvrir le fichier." << std::endl;
	       	}
	       	//ajouter les infos dans le file
	       	myfile << ActualRound << " " << Net_Device->GetMac()->GetShortAddress() << " " << params.m_srcAddr << " " << static_cast<int>(params.m_mpduLinkQuality) << " " << Simulator::Now ().GetSeconds() <<"\n";
			myfile.close();
			ClusterMembers.push_back(params.m_srcAddr);
		}
	}
}

void Leach::ReceiveClusterCreate (McpsDataIndicationParams params , Ptr<Packet> packet)
{
	ClusterCreateHeader CreateHeader;
	packet->RemoveHeader(CreateHeader);

	if (NodeType == 0)
	{
		CreateHeader.ClusterMembers = ClusterMembers;
	}
}

void Leach::CH_Characteristics ()
{
	//cout << "node " << GetNode()->GetId() << " is CH" <<endl;
	//cout << "at " << Simulator::Now ().GetSeconds () <<" s" <<endl;
	anim->UpdateNodeColor(GetNode()->GetId() , 0 , 255 , 0);
	anim->UpdateNodeSize(GetNode()->GetId() , 10 , 10);
}

void Leach::ResetApplication ()
{
	// Récupérer l'ID du node sur lequel l'app est installé
	uint32_t nodeId = GetNode()->GetId();
	// Changer la couleur 
	anim->UpdateNodeColor (nodeId, 255 , 0 , 0);
	anim->UpdateNodeSize(nodeId , 5 , 5);


	NodeType = 0;

	CH_Propositions.clear();			
	 				
	ClusterMembers.clear();					
}

void Leach::DeadNode ()
{
	//cout << "node "<< GetNode()->GetId() << " is dead at :" << Simulator::Now ().GetSeconds () <<" s" <<endl;
	// Désactive le nœud ou passe à un état d'économie d'énergie
	/*
	for (uint i = 0 ; i < GetNode()->GetNDevices() ; i++)
	{
		// Désactiver le périphérique de réseau
		GetNode()->GetDevice(i)->SetReceiveCallback(MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address&>());
	}
	*/
}

void Leach::RemEnergy ()
{
	/*
	cout << "node "<< GetNode()->GetId() << " has :" << endl;
	cout << GetNode()->GetObject<BasicEnergySource>()->GetRemainingEnergy() << endl;
	cout << "at " << Simulator::Now ().GetSeconds () <<" s" <<endl;
	*/
}

void Leach::StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState)
{
	/*
  NS_LOG_UNCOND (context << " state change at " << now.GetSeconds ()
                         << " from " << LrWpanHelper::LrWpanPhyEnumerationPrinter (oldState)
                         << " to " << LrWpanHelper::LrWpanPhyEnumerationPrinter (newState));
                         */
}

void Leach::StartApplication ()
{
	/*******************
	Initialiser l'affichage des Nodes au début de la simulation
	 									***********************/

	// Récupérer l'ID du node sur lequel l'app est installé
	uint32_t nodeId = GetNode()->GetId();
	// Changer la couleur 
	anim->UpdateNodeColor (nodeId, 255 , 0 , 0);
	//anim->UpdateNodeDescription (GetNode() , "Not Cluster Head yet");
	anim->UpdateNodeSize(nodeId , 5 , 5);

	Ptr<LrWpanNetDevice> Net_Device = DynamicCast<LrWpanNetDevice>(GetNode()->GetDevice(0));

	Net_Device->GetMac()->SetMcpsDataIndicationCallback(MakeCallback (&Leach::DataIndication , this));

	/*
	// Trace state changes in the phy
  	Net_Device->GetPhy ()->TraceConnect ("TrxState", std::string ("phy"), MakeCallback (&Leach::StateChangeNotification));



  	Ptr<SimpleDeviceEnergyModel> energyModel = GetNode()->GetObject<SimpleDeviceEnergyModel>();

  	// Configurer un rappel sur le début de la transmission d'un paquet
  	Net_Device->GetMac()->TraceConnectWithoutContext("Tx", MakeCallback(&SimpleDeviceEnergyModel::ChangeState, energyModel));

  	// Configurer un rappel sur le début de la réception d'un paquet
  	Net_Device->GetMac()->TraceConnectWithoutContext("Rx", MakeCallback(&SimpleDeviceEnergyModel::ChangeState, energyModel));
	*/

	// Planifier la première action à faire
	m_event = Simulator::Schedule(Seconds(0.0) , &Leach::Round, this);

}

void Leach::Round ()
{
	ResetApplication();
	ActualRound++;
	Generate_Random_Number ();
	Threshold ();
	if (isCH())
	{
		CH_Characteristics();
		SendClusterAdvertise();
		e_event = Simulator::Schedule (Seconds (5) , &Leach::SendClusterCreate , this);
	}
	//Simulator::Schedule (Seconds (Simulator::Now ().GetSeconds () + 9.99) , &Leach::ResetApplication , this);
	// Planifier la prochaine action
	m_event = Simulator::Schedule(Seconds(2500.0), &Leach::Round, this);
}

void Leach::StopApplication ()
{
	// Annuler tous les événements planifiés
  	Simulator::Cancel(m_event);
}

void Leach::DoDispose() 
{
	Application::DoDispose();
}

//===================== définition de la calsse Leach Helper =======================

//==================================================================================



class LeachHelper 
{
	public:
  		LeachHelper () 
  		{
    		m_factory.SetTypeId ("ns3::Leach");
  		}

  		void SetAttribute (std::string name, const ns3::AttributeValue &value) {
    		m_factory.Set (name, value);
  		}

  		ns3::ApplicationContainer Install (ns3::NodeContainer c) const 
  		{
   	 		ns3::ApplicationContainer apps;
    		for (ns3::NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) 
    		{
      			ns3::Ptr<ns3::Node> node = *i;
      			ns3::Ptr<Leach> app = m_factory.Create<Leach> ();
      			node->AddApplication (app);
      			apps.Add (app);
    		}	
    		return apps;
  		}

	private:
  		ns3::ObjectFactory m_factory;
};




void Set_Position(NodeContainer noeuds)
{
	// Initialisation d'un objet de positionnement
	Ptr<PositionAllocator> positionAlloc = CreateObject<RandomRectanglePositionAllocator> ();
	positionAlloc->SetAttribute ("X", StringValue ("ns3::UniformRandomVariable[Min=5|Max=950]"));
	positionAlloc->SetAttribute ("Y", StringValue ("ns3::UniformRandomVariable[Min=5|Max=950]"));

	// Création d'un objet MobilityHelper et ajout du modèle de positionnement
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel");
	mobility.SetPositionAllocator (positionAlloc);
	//mobility.Install (&n);


	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
	                           "Bounds", RectangleValue (Rectangle (0, 1000, 0, 1000)),
	                           "Distance", DoubleValue(50),
	                           "Speed" , StringValue ("ns3::ConstantRandomVariable[Constant=50]"));

	mobility.Install (noeuds);
}

void EnergyCallback(Ptr<Node> node, std::string context, double oldValue, double newValue)
{
	ofstream f;
	f.open ("Energy.txt", std::ofstream::out | std::ofstream::app);
	//ajouter les energy dans le file
	f << "node :" << node->GetId() << " at :" << Simulator::Now ().GetSeconds () <<" s"<<";\n";
	f << "has :" << newValue << "\n" ;
	f.close();
	if (!nodeIsDead[node])
	{
		if (newValue <= SEUIL)  // si le noeud n'est pas déjà mort
		{
			//node->GetObject<BasicEnergySource>()->SetInitialEnergy(0);
			cout << "node "<< node->GetId() << " is dead at :" << Simulator::Now ().GetSeconds () <<" s" <<endl;
			nodeIsDead[node] = true;  // Marque le noeud comme mort.

			// Vérifie si tous les nœuds sont morts.
			bool allNodesAreDead = true;
			for (const auto &pair : nodeIsDead) 
			{
				if (!pair.second) 
				{  // Si un nœud n'est pas mort...
					allNodesAreDead = false;
					break;  // ...arrête la vérification.
				}
			}

			// Arrêtez toutes les applications
			for (uint32_t i = 0; i < node->GetNApplications(); ++i)
			{
				Ptr<Leach> app = DynamicCast<Leach>(node->GetApplication(i));
				app->StopApplication();
			}

			/*
			// Désactivez toutes les interfaces réseau
			for (uint32_t i = 0; i < node->GetNDevices(); ++i)
			{
				Ptr<LrWpanNetDevice> device = DynamicCast<LrWpanNetDevice>(node->GetDevice(i));
				device->SetReceiveCallback(MakeNullCallback<bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address&>());
			}
			*/

			
			if (allNodesAreDead) 
			{
				cout << "All nodes are dead at :" << Simulator::Now ().GetSeconds () <<" s" <<endl;
				Simulator::Stop (Simulator::Now ());
			}
		}
	}
}


int main (int argc , char* argv[])
{	
	//Effacer le contenu du fichier 
	std::ofstream f1("Clusters.txt", std::ofstream::out | std::ofstream::trunc);
	f1.close();

	//Effacer le contenu du fichier 
	std::ofstream f2("Energy.txt", std::ofstream::out | std::ofstream::trunc);
	f2.close();

	//Effacer le contenu du fichier 
	std::ofstream f3("Packages.txt", std::ofstream::out | std::ofstream::trunc);
	f3.close();

	//Pour changer les parametres de la simulation 
	CommandLine cmd;
	cmd.AddValue ("nodes", "set the number of nodes", NUM_NODES);
	cmd.AddValue ("cluster_percentage", "set the percentage of clusters per round", CLUSTER_PERCENTAGE);

	cmd.Parse (argc, argv);

	// Pour Randomiser les simulations
	SeedManager::SetSeed (time (NULL));

	// Pour Randomiser les nombres aléatoires avec rand()
	srand (time(NULL));

	// Création d'un container pour les noeuds
	NodeContainer nodes;
	//Créer un réseau de NUM_NODES nodes
	nodes.Create(NUM_NODES);

	Set_Position(nodes);

	//======================================802.15.4===============================================

	// Configurer le modèle de propagation

	Ptr<RandomPropagationLossModel> loss = CreateObject<RandomPropagationLossModel> ();

 	Ptr<PropagationDelayModel> delay = CreateObject<RandomPropagationDelayModel> ();

   	/************************
    *  Create the helpers  *
    ************************/
   	LrWpanHelper lrWpanHelper;

	Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
	channel->SetPropagationDelayModel(delay);
	channel->AddPropagationLossModel (loss);

	// Utilisez le canal dans le LrWpanHelper
	lrWpanHelper.SetChannel(channel);

	// Installation du standard 802.15.4 sur les nœuds
	NetDeviceContainer devices = lrWpanHelper.Install(nodes);

	/*
	Dans ce code, une boucle est utilisée pour générer une adresse MAC unique
	pour chaque nœud dans le réseau.
	La partie std::hex << i+1 de os << "00:0" << std::hex << i+1;
	convertit la valeur de i+1 en hexadécimal, 
	qui est ensuite utilisée pour créer une adresse MAC.
	Le i+1 est utilisé pour éviter d'avoir une adresse avec un zéro en position finale,
	car 00:00 n'est pas une adresse MAC valide.
	*/

	// Configurer l'adressage pour chaque node
	for(uint32_t i=0 ; i < nodes.GetN() ; i++) 
	{
	    std::ostringstream os;
	    os << "00:0" << std::hex << i+1;
	    Mac16Address addr(os.str().c_str()); 
	    devices.Get(i)->SetAddress(addr);

	    network[addr] = nodes.Get(i);
	}

	for (uint i = 0 ; i < nodes.GetN() ; ++i)
	{
		Ptr<BasicEnergySource> energySource = CreateObject<BasicEnergySource>();
		// Configurer les paramètres de la source d'énergie Li-ion 
		energySource->SetInitialEnergy(MIN + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX/(MAX-MIN))));
		nodes.Get(i)->AggregateObject(energySource);
		energySource->SetEnergyUpdateInterval(Seconds(0.1));
		// Connecte la trace à la fonction de rappel
		energySource->TraceConnect("RemainingEnergy", std::string(), MakeBoundCallback(&EnergyCallback, nodes.Get(i)));
	}

	for (uint i = 0 ; i < nodes.GetN() ; i++)
	{
		// Device energy model.
		//WifiRadioEnergyModelHelper Model;
	  	Ptr<WifiRadioEnergyModel> Model = CreateObject<WifiRadioEnergyModel> ();
	  	// configure radio energy model
		Model->SetIdleCurrentA (0.000001);
		Model->SetCcaBusyCurrentA (0.0031);
		Model->SetTxCurrentA (0.0085);
		Model->SetRxCurrentA (0.0064);
	  	//simpleModel->SetTxCurrentA(0.0174);
		//simpleModel->SetRxCurrentA(0.0174);
	  	Model->SetEnergySource (nodes.Get(i)->GetObject<BasicEnergySource>());
	  	(nodes.Get(i)->GetObject<BasicEnergySource>())->AppendDeviceEnergyModel (Model);
	  	//simpleModel->SetCurrentA(0.0173);
	}

	for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i) 
	{
		nodeIsDead[*i] = false;  // Initialise tous les nœuds comme étant "vivants".
	}

	/*
	BasicEnergySourceHelper basicSourceHelper;
	basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (10.0));
	EnergySourceContainer sources = basicSourceHelper.Install (nodes);

	// Créer un SimpleDeviceEnergyModel pour chaque source d'énergie
	for (EnergySourceContainer::Iterator iter = sources.Begin (); iter != sources.End (); iter++)
	{
	  	Ptr<SimpleDeviceEnergyModel> simpleModel = CreateObject<SimpleDeviceEnergyModel> ();
	  	simpleModel->SetEnergySource (*iter);
	  	(*iter)->AppendDeviceEnergyModel (simpleModel);
	}
	*/
	


	LeachHelper LH;
	ApplicationContainer Apps = LH.Install(nodes);

	Apps.Start (Seconds (0.0));
	Apps.Stop (Seconds (duration));

	anim = new AnimationInterface ("Leach.xml");

	Simulator::Stop (Seconds (duration));
    Simulator::Run ();
    Simulator::Destroy ();

	return 0;
}
