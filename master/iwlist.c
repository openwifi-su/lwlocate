/*
 *	Wireless Tools
 *
 *		Jean II - HPLB '99 - HPL 99->07
 *
 * This tool can access various piece of information on the card
 * not part of iwconfig...
 * You need to link this code against "iwlist.c" and "-lm".
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2007 Jean Tourrilhes <jt@hpl.hp.com>
 */

#include "libwlocate.h"
#include "iwlib.h"		/* Header */
#include <sys/time.h>

static struct wloc_req    *g_request;
static struct iwscan_state state;

/****************************** TYPES ******************************/

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
  /* State */
  int			ap_num;		/* Access Point number 1->N */
  int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

/*
 * Bit to name mapping
 */
typedef struct iwmask_name
{
  unsigned int	mask;	/* bit mask for the value */
  const char *	name;	/* human readable name for the value */
} iwmask_name;

/*
 * Types of authentication parameters
 */
typedef struct iw_auth_descr
{
  int				value;		/* Type of auth value */
  const char *			label;		/* User readable version */
  const struct iwmask_name *	names;		/* Names for this value */
  const int			num_names;	/* Number of names */
} iw_auth_descr;

/**************************** CONSTANTS ****************************/

#define IW_SCAN_HACK		0x8000

#define IW_EXTKEY_SIZE	(sizeof(struct iw_encode_ext) + IW_ENCODING_TOKEN_MAX)



/***************************** SCANNING *****************************/
/*
 * This one behave quite differently from the others
 *
 * Note that we don't use the scanning capability of iwlib (functions
 * iw_process_scan() and iw_scan()). The main reason is that
 * iw_process_scan() return only a subset of the scan data to the caller,
 * for example custom elements and bitrates are ommited. Here, we
 * do the complete job...
 */

/*------------------------------------------------------------------*/
/*
 * Print one element from the scanning results
 */
static inline void
print_scanning_token(struct stream_descr *	stream,	/* Stream of events */
		     struct iw_event *		event,	/* Extracted token */
		     struct iwscan_state *	state,
		     struct iw_range *	iw_range,	/* Range info */
		     int		has_range)
{
   int  i;
   char buffer[128];

   if (state->ap_num>=WLOC_MAX_NETWORKS)
	return;
  /* Now, let's decode the event */
  switch(event->cmd)
    {
    case SIOCGIWAP:
      printf("          Cell %02d - Address: %s\n", state->ap_num,
	     iw_saether_ntop(&event->u.ap_addr, buffer));
      state->ap_num++;
      for (i=0; i<6; i++)
       g_request->bssids[state->ap_num-1][i]=(event->u.ap_addr.sa_data[i] & 0xFF);
      break;
    case IWEVQUAL:
    {
      if (iw_range->max_qual.qual==0) g_request->signal[state->ap_num-1]=abs(event->u.qual.qual);
      else g_request->signal[state->ap_num-1]=100.0*event->u.qual.qual/iw_range->max_qual.qual;
      printf("                    Signal: %d\n",g_request->signal[state->ap_num-1]);
      break;
    }
    default:
      break;
   }	/* switch(event->cmd) */
}

/*------------------------------------------------------------------*/
/*
 * Perform a scanning on one device
 */
static int
print_scanning_info(int		skfd,
		    char *	ifname,
		    char *	args[],		/* Command line args */
		    int		count)		/* Args count */
{
  struct iwreq		wrq;
  struct iw_scan_req    scanopt;		/* Options for 'set' */
  int			scanflags = 0;		/* Flags for scan */
  unsigned char *	buffer = NULL;		/* Results */
  int			buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
  struct iw_range	range;
  int			has_range;
  struct timeval	tv;				/* Select timeout */
  int			timeout = 15000000;		/* 15s */

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  /* Debugging stuff */
/*  if((IW_EV_LCP_PK2_LEN != IW_EV_LCP_PK_LEN) || (IW_EV_POINT_PK2_LEN != IW_EV_POINT_PK_LEN))
    {
      fprintf(stderr, "*** Please report to jt@hpl.hp.com your platform details\n");
      fprintf(stderr, "*** and the following line :\n");
      fprintf(stderr, "*** IW_EV_LCP_PK2_LEN = %zu ; IW_EV_POINT_PK2_LEN = %zu\n\n",
	      IW_EV_LCP_PK2_LEN, IW_EV_POINT_PK2_LEN);
    }*/

  /* Get range stuff */
  has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);

  /* Check if the interface could support scanning. */
  if (range.we_version_compiled==0) range.we_version_compiled=29;
  if((!has_range) || (range.we_version_compiled < 14))
    {
      fprintf(stderr, "%-8.16s  Interface doesn't support scanning.\n\n",
	      ifname);
      return(-1);
    }

  /* Init timeout value -> 250ms between set and first get */
  tv.tv_sec = 0;
  tv.tv_usec = 250000;

  /* Clean up set args */
  memset(&scanopt, 0, sizeof(scanopt));

  /* Parse command line arguments and extract options.
   * Note : when we have enough options, we should use the parser
   * from iwconfig... */
/*  while(count > 0)
    {

      count--;
      
      if(!strncmp(args[0], "essid", 5))
	{
	  if(count < 1)
	    {
	      fprintf(stderr, "Too few arguments for scanning option [%s]\n",
		      args[0]);
	      return(-1);
	    }
	  args++;
	  count--;

	  scanopt.essid_len = strlen(args[0]);
	  memcpy(scanopt.essid, args[0], scanopt.essid_len);
	  if(scanopt.bssid.sa_family == 0)
	    {
	      scanopt.bssid.sa_family = ARPHRD_ETHER;
	      memset(scanopt.bssid.sa_data, 0xff, ETH_ALEN);
	    }
	  scanflags |= IW_SCAN_THIS_ESSID;
	}
      else
	if(!strncmp(args[0], "last", 4))
	  {
	    scanflags |= IW_SCAN_HACK;
	  }
	else
	  {
	    fprintf(stderr, "Invalid scanning option [%s]\n", args[0]);
	    return(-1);
	  }

      args++;
    }*/

  /* Check if we have scan options */
/*  if(scanflags)
    {
      wrq.u.data.pointer = (caddr_t) &scanopt;
      wrq.u.data.length = sizeof(scanopt);
      wrq.u.data.flags = scanflags;
    }
  else*/
    {
      wrq.u.data.pointer = NULL;
      wrq.u.data.flags = 0;
      wrq.u.data.length = 0;
    }

  /* If only 'last' was specified on command line, don't trigger a scan */
/*  if(scanflags == IW_SCAN_HACK)
    {
      tv.tv_usec = 0;
    }
  else*/
    {
      /* Initiate Scanning */
      if(iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0)
	{
	  if((errno != EPERM) || (scanflags != 0))
	    {
	      fprintf(stderr, "%-8.16s  Interface doesn't support scanning : %s\n\n",
		      ifname, strerror(errno));
	      return(-1);
	    }
	  /* If we don't have the permission to initiate the scan, we may
	   * still have permission to read left-over results.
	   * But, don't wait !!! */
#if 0
	  /* Not cool, it display for non wireless interfaces... */
	  fprintf(stderr, "%-8.16s  (Could not trigger scanning, just reading left-over results)\n", ifname);
#endif
	  tv.tv_usec = 0;
	}
    }
  timeout -= tv.tv_usec;

  /* Forever */
  while(1)
    {
      fd_set		rfds;		/* File descriptors for select */
      int		last_fd;	/* Last fd */
      int		ret;

      /* Guess what ? We must re-generate rfds each time */
      FD_ZERO(&rfds);
      last_fd = -1;

      /* In here, add the rtnetlink fd in the list */

      /* Wait until something happens */
      ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);

      /* Check if there was an error */
      if(ret < 0)
	{
	  if(errno == EAGAIN || errno == EINTR)
	    continue;
	  fprintf(stderr, "Unhandled signal - exiting...\n");
	  return(-1);
	}

      /* Check if there was a timeout */
      if(ret == 0)
	{
	  unsigned char *	newbuf;

	realloc:
	  /* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
	  newbuf = (unsigned char*)realloc(buffer, buflen);
	  if(newbuf == NULL)
	    {
	      if(buffer)
		free(buffer);
	      fprintf(stderr, "%s: Allocation failed\n", __FUNCTION__);
	      return(-1);
	    }
	  buffer = newbuf;

	  /* Try to read the results */
	  wrq.u.data.pointer = buffer;
	  wrq.u.data.flags = 0;
	  wrq.u.data.length = buflen;
	  if(iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0)
	    {
	      /* Check if buffer was too small (WE-17 only) */
	      if((errno == E2BIG) && (range.we_version_compiled > 16))
		{
		  /* Some driver may return very large scan results, either
		   * because there are many cells, or because they have many
		   * large elements in cells (like IWEVCUSTOM). Most will
		   * only need the regular sized buffer. We now use a dynamic
		   * allocation of the buffer to satisfy everybody. Of course,
		   * as we don't know in advance the size of the array, we try
		   * various increasing sizes. Jean II */

		  /* Check if the driver gave us any hints. */
		  if(wrq.u.data.length > buflen)
		    buflen = wrq.u.data.length;
		  else
		    buflen *= 2;

		  /* Try again */
		  goto realloc;
		}

	      /* Check if results not available yet */
	      if(errno == EAGAIN)
		{
		  /* Restart timer for only 100ms*/
		  tv.tv_sec = 0;
		  tv.tv_usec = 100000;
		  timeout -= tv.tv_usec;
		  if(timeout > 0)
		    continue;	/* Try again later */
		}

	      /* Bad error */
	      free(buffer);
	      fprintf(stderr, "%-8.16s  Failed to read scan data : %s\n\n",
		      ifname, strerror(errno));
	      return(-2);
	    }
	  else
	    /* We have the results, go to process them */
	    break;
	}

      /* In here, check if event and event type
       * if scan event, read results. All errors bad & no reset timeout */
    }

  if(wrq.u.data.length)
    {
      struct iw_event		iwe;
      struct stream_descr	stream;
      int			ret;
      
      state.ap_num = 0;
      state.val_index = 0;
#ifdef DEBUG
      /* Debugging code. In theory useless, because it's debugged ;-) */
      int	i;
      printf("Scan result %d [%02X", wrq.u.data.length, buffer[0]);
      for(i = 1; i < wrq.u.data.length; i++)
	printf(":%02X", buffer[i]);
      printf("]\n");
#endif
      printf("%-8.16s  Scan completed :\n", ifname);
      iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
      do
	{
	  /* Extract an event and print it */
	  ret = iw_extract_event_stream(&stream, &iwe,
					range.we_version_compiled);
	  if(ret > 0)
	    print_scanning_token(&stream, &iwe, &state,
				 &range, has_range);
	}
      while(ret > 0);
      printf("\n");
    }
  else
    printf("%-8.16s  No scan results\n\n", ifname);

  free(buffer);
  return(0);
}

/*********************** FREQUENCIES/CHANNELS ***********************/

/*------------------------------------------------------------------*/
/*
 * Power Management types of values
 */
static const unsigned int pm_type_flags[] = {
  IW_POWER_PERIOD,
  IW_POWER_TIMEOUT,
  IW_POWER_SAVING,
};
static const int pm_type_flags_size = (sizeof(pm_type_flags)/sizeof(pm_type_flags[0]));



/************************* COMMON UTILITIES *************************/
/*
 * This section was initially written by Michael Tokarev <mjt@tls.msk.ru>
 * but heavily modified by me ;-)
 */

/*------------------------------------------------------------------*/
/*
 * Map command line arguments to the proper procedure...
 */
typedef struct iwlist_entry {
  const char *		cmd;		/* Command line shorthand */
  iw_enum_handler	fn;		/* Subroutine */
  int			max_count;
  const char *		argsname;	/* Args as human readable string */
} iwlist_cmd;

static const struct iwlist_entry iwlist_cmds[] = {
  { "scanning",		print_scanning_info,	-1, "[essid NNN] [last]" },
  { NULL, NULL, 0, 0 },
};

/*------------------------------------------------------------------*/
/*
 * Find the most appropriate command matching the command line
 */
static inline const iwlist_cmd *
find_command(const char *	cmd)
{
  const iwlist_cmd *	found = NULL;
  int			ambig = 0;
  unsigned int		len = strlen(cmd);
  int			i;

  /* Go through all commands */
  for(i = 0; iwlist_cmds[i].cmd != NULL; ++i)
    {
      /* No match -> next one */
      if(strncasecmp(iwlist_cmds[i].cmd, cmd, len) != 0)
	continue;

      /* Exact match -> perfect */
      if(len == strlen(iwlist_cmds[i].cmd))
	return &iwlist_cmds[i];

      /* Partial match */
      if(found == NULL)
	/* First time */
	found = &iwlist_cmds[i];
      else
	/* Another time */
	if (iwlist_cmds[i].fn != found->fn)
	  ambig = 1;
    }

  if(found == NULL)
    {
      fprintf(stderr, "iwlist: unknown command `%s' (check 'iwlist --help').\n", cmd);
      return NULL;
    }

  if(ambig)
    {
      fprintf(stderr, "iwlist: command `%s' is ambiguous (check 'iwlist --help').\n", cmd);
      return NULL;
    }

  return found;
}



int iw_fill_structure(struct wloc_req *request)
{
  int skfd;
  /* Create a channel to the NET kernel. */
  if((skfd = iw_sockets_open()) < 0)
    {
      perror("socket");
      return -1;
    }
  g_request=request;

  /* do the actual work */
  iw_enum_devices(skfd,print_scanning_info,NULL,-1);

  /* Close the socket. */
  iw_sockets_close(skfd);

  return state.ap_num;
}
