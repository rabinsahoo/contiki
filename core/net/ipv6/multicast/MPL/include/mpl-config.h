#ifndef __MPL_CONFIG_H__
#define __MPL_CONFIG_H__

/*Indicates whether or not the MPL forwarder schedules MPL data message transmission
  after receving them for the first time. This configuration will be true by default
  All MPL interface on the same link SHOULD configure same value. We can vary this
  configuration across the MPL interface on the same link if reactive forwarding is
  also used.
*/
#ifdef MPL_CONF_PROACTVE_FORWARDING
#define MPL_PROACTIVE_FORWARDING MPL_CONF_PROACTVE_FORWARDING
#else
#define MPL_PROACTIVE_FORWARDING 1
#endif

/* Defines the minimum lifetime for an entry in the seed set. SEED_SET_ENTRY_LIFETIME
   has a deafult value of 30 minutes. Its recomended that all MPL forwarder use the 
   same value for this configuration for a a given MPL domain and use the default 
   value of 30 minutes. Configuring too small lifetime can cause the duplicate 
   detection mechanism to fail.
*/
#ifdef MPL_CONF_SEED_SET_ENTRY_LIFETIME
#define MPL_SEED_SET_ENTRY_LIFETIME MPL_CONF_SEED_SET_ENTRY_LIFETIME
#else
#define MPL_SEED_SET_ENTRY_LIFETIME 30 /* Its in Minute */
#endif

/* Trickle parameters Imin, Imax and K.
   Defines the minimum trickle timer interval for MPL data message transmission. It
   has a default value of 10 times the expected link-layer latency.
   We will set this value in terms of (lg2N) where N is the total interval.
   We have considered the link latency to be 10ms so with 10 times it will be 
   100ms.
*/
#ifdef MPL_CONF_DATA_MESSAGE_IMIN
#define MPL_DATA_MESSAGE_IMIN MPL_CONF_DATA_MESSAGE_IMIN
#else
#define MPL_DATA_MESSAGE_IMIN 7 
#endif

/*Defines the maximum trickle timer interval for MPL data message transmision.
  It has a default value equal to DATA_MESSAGE_IMIN.
  As per trickle algorithm Imax is described as a number of doubling of of the 
  minimum interval size.
*/
#ifdef MPL_CONF_DATA_MESSAGE_INTVL_DBLING
#define MPL_DATA_MESSAGE_INTVL_DBLING MPL_CONF_DATA_MESSAGE_INTVL_DBLING
#else 
#define MPL_DATA_MESSAGE_INTVL_DBLING 12
#endif

/*The redundancy constant for MPL data message transmission. It has a default value of 1*/
#ifdef MPL_CONF_DATA_MESSAGE_K
#define MPL_DATA_MESSAGE_K MPL_CONF_DATA_MESSAGE_K
#else
#define MPL_DATA_MESSAGE_K 1
#endif

/*Defines the number of trickle timer expirations that occur before terminating the
  trickle algorithm's retransmissions of a given MPL data message. It has a default
  value of 3*/
#ifdef MPL_CONF_DATA_MESSAGE_TIMER_EXPIRATIONS
#define MPL_DATA_MESSAGE_TIMER_EXPIRATIONS MPL_CONF_DATA_MESSAGE_TIMER_EXPIRATIONS
#else
#define MPL_DATA_MESSAGE_TIMER_EXPIRATIONS 3
#endif

/*Defines the minimum trickle timer interval for MPL control message transmission.
  Default value is 10 times the worst-case link layer latency
*/
#ifdef MPL_CONF_CONTROL_MESSAGE_IMIN
#define MPL_CONTROL_MESSAGE_IMIN MPL_CONF_CONTROL_MESSAGE_IMIN
#else
#define MPL_CONTROL_MESSAGE_IMIN 8 /*May be in ms*/
#endif

/*Defines maximum trickle timer interval for MPL control message transmission.
  Default value is 5 minute.
*/
#ifdef MPL_CONF_CONTROL_MESSAGE_INTVL_DBLING
#define MPL_CONTROL_MESSAGE_INTVL_DBLING MPL_CONF_CONTROL_MESSAGE_INTVL_DBLING
#else
#define MPL_CONTROL_MESSAGE_INTVL_DBLING 12 /* Its in Minute */
#endif

/*Defines the redundancy constant for MPL control message transmission. The default
  value is 1
*/
#ifdef MPL_CONF_CONTROL_MESSAGE_K
#define MPL_CONTROL_MESSAGE_K MPL_CONF_CONTROL_MESSAGE_K
#else
#define MPL_CONTROL_MESSAGE_K 1
#endif

/*Define the number of trickle timer expirations that occur before terminating the
  trickle algorithm's retransmission of a give RPL control message. It has a default
  value of 10 
*/
#ifdef MPL_CONF_CONTROL_MESSAGE_TIMER_EXPIRATIONS
#define MPL_CONTROL_MESSAGE_TIMER_EXPIRATIONS MPL_CONF_CONTROL_MESSAGE_TIMER_EXPIRATIONS
#else
#define MPL_CONTROL_MESSAGE_TIMER_EXPIRATIONS 10
#endif


/* Define the IPv6 MTU Size */
#ifdef MPL_CONF_IPV6_MTU
#define MPL_IPV6_MTU MPL_CONF_IPV6_MTU
#else
#define MPL_IPV6_MTU 1280
#endif

#ifdef MPL_CONF_NUM_MPL_DOMAINS
#define MPL_NUM_MPL_DOMAINS MPL_CONF_NUM_MPL_DOMAINS
#else
#define MPL_NUM_MPL_DOMAINS 1
#endif

#ifdef MPL_CONF_SEED_SET_SIZE
#define MPL_SEED_SET_SIZE MPL_CONF_SEED_SET_SIZE
#else
#define MPL_SEED_SET_SIZE 10
#endif

#ifdef MPL_BUFFERED_MESSAGE_SET_CB_SIZE
#define MPL_BUFFERED_MESSAGE_SET_CB_SIZE MPL_BUFFERED_MESSAGE_SET_CB_SIZE
#else
#define MPL_BUFFERED_MESSAGE_SET_CB_SIZE 64
#endif

#endif /*__MPL_CONFIG_H__*/


