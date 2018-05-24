#include <iostream>

#include "controller.hh"
#include "timestamp.hh"
#include <math.h>

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << cwnd + dwnd << endl;
  }

  return cwnd + dwnd;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  /* Default: take no action */
  if (after_timeout) {
    /* revert to slow start */ 
    cwnd_ = cwnd = 1;
    dwnd_ = dwnd = 0;
    slow_start = true;
  }

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }
}

void Controller::update_rtt(const uint64_t timestamp_ack_received, 
                               const uint64_t send_timestamp_acked) {

  double cur_rtt = double(timestamp_ack_received - send_timestamp_acked);
  rtt = rtt_smooth * cur_rtt + (1 - rtt_smooth) * rtt;
  if (base_rtt > cur_rtt)
    base_rtt = cur_rtt;
  if (debug_)
    cerr << "rtt: " << rtt << endl; 
}

void Controller::update_dwnd(double win, double diff) {
  /* for now, assume network is not lossy. */
  double update;
  if (diff < gamma) {
    update = alpha * win - 1; /* TODO: update to pow(win, k)*/
    if (update < 0) update = 0;
    dwnd_ += update;
  } else {
    dwnd_ = zeta * diff; 
    if (dwnd_ < 0) dwnd_ = 0;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{

  /* for now, assume network is not lossy. */
  recv_seqno = max(sequence_number_acked, recv_seqno);
  update_rtt(timestamp_ack_received, send_timestamp_acked);

  if (slow_start) {
    cwnd += 1;
    if (rtt > SLOWSTART_TIMEOUT)
      slow_start = false; 
    cwnd_ = cwnd;
    dwnd_ = dwnd;
  } else {
    /* update cwnd according to normal tcp: */
    cwnd_ += 1/(cwnd_ + dwnd_);
    
    double win = cwnd_ + dwnd_;
    double expected = win / base_rtt;
    double actual =  win / rtt;
    double diff = (expected - actual) * base_rtt;
    update_dwnd(win, diff);

    cwnd = int(cwnd_);    
    dwnd = int(dwnd_);
  } 

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return 500; /* timeout of one second */
}
