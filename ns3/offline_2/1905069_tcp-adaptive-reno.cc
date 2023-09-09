/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-adaptive-reno.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "rtt-estimator.h"
#include "tcp-socket-base.h"
using namespace std;

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpAdaptiveReno")
            .SetParent<TcpNewReno>()
            .SetGroupName("Internet")
            .AddConstructor<TcpAdaptiveReno>()
            .AddAttribute(
                "FilterType",
                "Use this to choose no filter or Tustin's approximation filter",
                EnumValue(TcpAdaptiveReno::TUSTIN),
                MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
            .AddTraceSource("EstimatedBW",
                            "The estimated bandwidth",
                            MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                            "ns3::TracedValueCallback::DataRate");
    return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno()
    : TcpWestwoodPlus(),
    m_rttMin(Time(0)),
    m_rtt(Time(0)),
    m_a(0),
    m_rttCongLast(Time(0)),
    m_rttCongCurr(Time(0)),
    m_rttJthPacketLost(Time(0)),
    m_WInc(0),
    m_WBase(0),
    m_WProbe(0)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& sock)
    : TcpWestwoodPlus(sock),
    m_rttMin(sock.m_rttMin),
    m_rtt(sock.m_rtt),
    m_a(sock.m_a),
    m_rttCongLast(sock.m_rttCongLast),
    m_rttCongCurr(sock.m_rttCongCurr),
    m_rttJthPacketLost(sock.m_rttJthPacketLost),
    m_WInc(sock.m_WInc),
    m_WBase(sock.m_WBase),
    m_WProbe(sock.m_WProbe)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("Invoked the copy constructor");
}

TcpAdaptiveReno::~TcpAdaptiveReno()
{
}

void
TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

    if (rtt.IsZero())
    {
        NS_LOG_WARN("RTT measured is zero!");
        return;
    }

    m_ackedSegments += packetsAcked;

    if(m_rttMin.IsZero() || rtt<=m_rttMin) m_rttMin = rtt;
    m_rtt = rtt;
    TcpWestwoodPlus::EstimateBW (rtt, tcb);

        // if (!(rtt.IsZero() || m_IsCount))
        // {
        //     m_IsCount = true;
        //     m_bwEstimateEvent.Cancel();
        //     m_bwEstimateEvent = Simulator::Schedule(rtt, &TcpAdaptiveReno::EstimateBW, this, rtt, tcb);
        // }
}

double TcpAdaptiveReno::EstimateCongestionLevel(){
    m_a = 0.85;
    if(m_rttCongLast < m_rttMin) m_a = 0;
    m_rttCongCurr = Seconds(m_a*m_rttCongLast.GetSeconds() + (1-m_a)*m_rttJthPacketLost.GetSeconds());
    return min(1.0, (m_rtt.GetSeconds() - m_rttMin.GetSeconds())/(m_rttCongCurr.GetSeconds() - m_rttMin.GetSeconds()));
}

void TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb){
    double m_M = 1000;
    ostringstream os;
    os<< m_currentBW;
    double bandwidth = stod(os.str());
    double mss = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize);
    double m_WMaxInc =  bandwidth*mss/m_M;
    double alpha = 10;
    double beta = 2*m_WMaxInc*((1/alpha)-((1/alpha + 1.0)/exp(alpha)));
    double gamma = 1.0 - 2*m_WMaxInc*((1/alpha)-((1/alpha + 0.5)/exp(alpha)));
    double cong = EstimateCongestionLevel();
    m_WInc = ((m_WMaxInc/exp(alpha*cong)) + (beta*cong) + gamma);
}

void
TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);

    if (segmentsAcked > 0)
    {
        EstimateIncWnd(tcb);
        double adder = static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get();
        adder = std::max(1.0, adder);
        m_WBase += adder;
        m_WProbe = max((m_WProbe + m_WInc/tcb->m_cWnd.Get()),0.0);
        tcb->m_cWnd = m_WBase + m_WProbe;
        NS_LOG_INFO("In CongAvoid, updated to cwnd " << tcb->m_cWnd << " ssthresh "
                                                     << tcb->m_ssThresh);
    }
}



uint32_t
TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight [[maybe_unused]])
{

    m_rttCongLast = m_rttCongCurr;
    m_rttJthPacketLost = m_rtt;

    double cong = EstimateCongestionLevel();
    
    uint32_t ssthresh = (uint32_t)max(2*tcb->m_segmentSize,(uint32_t) (tcb->m_cWnd / (1.0+cong)));
    m_WBase = (double) ssthresh;
    m_WProbe = 0;

    //uint32_t ssThresh = static_cast<uint32_t>((m_currentBW * tcb->m_minRtt) / 8.0);

    NS_LOG_LOGIC("new ssthresh : "<<ssthresh<<" ; old conj Rtt : "<<m_rttCongLast<<" ; new conj Rtt : "<<m_rttCongCurr<<" ; cong : "<<cong);

    return ssthresh;
}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork()
{
    return CreateObject<TcpAdaptiveReno>(*this);
}

} // namespace ns3
