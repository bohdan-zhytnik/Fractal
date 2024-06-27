/*
 * Filename: messages.c
 * Date:     2017/04/15 11:22
 * Author:   Jan Faigl
 */

#include <string.h>
#include <stdio.h>

#include "messages.h"
#include "computation.h"
#include "utils.h"
// - function  ----------------------------------------------------------------
bool get_message_size(uint8_t msg_type, int *len)
{
   bool ret = true;
   switch(msg_type) {
      case MSG_OK:
      case MSG_QUIT:
      case MSG_ERROR:
      case MSG_ABORT:
      case MSG_DONE:
      case MSG_GET_VERSION:
         *len = 2; // 2 bytes message - id + cksum
         break;
      case MSG_STARTUP:
         *len = 2 + STARTUP_MSG_LEN;
         break;
      case MSG_VERSION:
         *len = 2 + 3 * sizeof(uint8_t); // 2 + major, minor, patch
         break;
      case MSG_SET_COMPUTE:
         *len = 2 + 4 * sizeof(double) + 1; // 2 + 4 * params + n 
         break;
      case MSG_COMPUTE:
         *len = 2 + 1 + 2 * sizeof(double) + 2; // 2 + cid (8bit) + 2x(double - re, im) + 2 ( n_re, n_im)
         break;
      case MSG_COMPUTE_DATA:
         *len = 2 + 4; // cid, dx, dy, iter
         break;
      case MSG_COMP_BURST:
         *len = 2 + sizeof(uint8_t) + 4 * sizeof(double) + sizeof(uint8_t*);
         break;
      case MSG_SET_COMP_BURST:
         *len = 2 + sizeof(double);
         break;
      case MSG_COMPUTE_DATA_BURST:
         *len = 2 + sizeof(uint8_t) * get_size_iters();
         break;
      default:
         ret = false;
         break;
   }
   return ret;
}

// - function  ----------------------------------------------------------------
bool fill_message_buf(const message *msg, uint8_t *buf, int size, int *len)
{
   if (!msg || size < sizeof(message) || !buf) {
      return false;
   }
   // 1st - serialize the message into a buffer
   bool ret = true;
   *len = 0;
   switch(msg->type) {
      case MSG_OK:
      case MSG_QUIT:
      case MSG_ERROR:
      case MSG_ABORT:
      case MSG_DONE:
      case MSG_GET_VERSION:
         *len = 1;
         break;
      case MSG_STARTUP:
         for (int i = 0; i < STARTUP_MSG_LEN; ++i) {
            buf[i+1] = msg->data.startup.message[i];
         }
         *len = 1 + STARTUP_MSG_LEN;
         break;
      case MSG_VERSION:
         buf[1] = msg->data.version.major;
         buf[2] = msg->data.version.minor;
         buf[3] = msg->data.version.patch;
         *len = 4;
         break;
      case MSG_SET_COMPUTE:
         memcpy(&(buf[1 + 0 * sizeof(double)]), &(msg->data.set_compute.c_re), sizeof(double));
         memcpy(&(buf[1 + 1 * sizeof(double)]), &(msg->data.set_compute.c_im), sizeof(double));
         memcpy(&(buf[1 + 2 * sizeof(double)]), &(msg->data.set_compute.d_re), sizeof(double));
         memcpy(&(buf[1 + 3 * sizeof(double)]), &(msg->data.set_compute.d_im), sizeof(double));
         buf[1 + 4 * sizeof(double)] = msg->data.set_compute.n;
         *len = 1 + 4 * sizeof(double) + 1;
         break;
      case MSG_COMPUTE:
         buf[1] = msg->data.compute.cid; // cid
         memcpy(&(buf[2 + 0 * sizeof(double)]), &(msg->data.compute.re), sizeof(double));
         memcpy(&(buf[2 + 1 * sizeof(double)]), &(msg->data.compute.im), sizeof(double));
         buf[2 + 2 * sizeof(double) + 0] = msg->data.compute.n_re;
         buf[2 + 2 * sizeof(double) + 1] = msg->data.compute.n_im;
         *len = 1 + 1 + 2 * sizeof(double) + 2;
         break;
      case MSG_COMPUTE_DATA:
         buf[1] = msg->data.compute_data.cid;
         buf[2] = msg->data.compute_data.i_re;
         buf[3] = msg->data.compute_data.i_im;
         buf[4] = msg->data.compute_data.iter;
         *len = 5;
         break;
      case MSG_COMP_BURST:
         buf[1] = msg->data.compute_data.cid;
         memcpy(&(buf[2 + 0 * sizeof(double)]), &(msg->data.comp_BURST.re), sizeof(double));
         memcpy(&(buf[2 + 1 * sizeof(double)]), &(msg->data.comp_BURST.im), sizeof(double));
         memcpy(&(buf[2 + 2 * sizeof(double)]), &(msg->data.comp_BURST.length), sizeof(double));
         memcpy(&(buf[2 + 3 * sizeof(double)]), &(msg->data.comp_BURST.iters), sizeof(msg->data.comp_BURST.iters));
         memcpy(&(buf[2 + 4* sizeof(double)]), &(msg->data.comp_BURST.grid_w), sizeof(double));
         *len = 1 + 1 + 4 *sizeof(double) + sizeof(uint8_t*);
         break;
      case MSG_SET_COMP_BURST:
         memcpy(&(buf[1 + 0 * sizeof(double)]), &(msg->data.set_comp_BURST), sizeof(double));
         break;
      case MSG_COMPUTE_DATA_BURST:
         memcpy(buf, msg->data.compute_data_BURST.iters, get_size_iters() * sizeof(uint8_t));
         *len =1+ get_size_iters() * sizeof(uint8_t);
         break;
      default: // unknown message type
         ret = false;
         break;
   }
   // 2nd - send the message buffer
   if (ret) { // message recognized
      buf[0] = msg->type;
      buf[*len] = 0; // cksum
      for (int i = 0; i < *len; ++i) {
         buf[*len] += buf[i];
      }
      buf[*len] = 255 - buf[*len]; // compute cksum
      *len += 1; // add cksum to buffer
   }
   return ret;
}

// - function  ----------------------------------------------------------------
bool parse_message_buf(const uint8_t *buf, int size, message *msg)
{
   uint8_t cksum = 0;
   for (int i = 0; i < size; ++i) {
      cksum += buf[i];
   }
   bool ret = false;
   int message_size;
   if (
         size > 0 && cksum == 0xff && // sum of all bytes must be 255
         ((msg->type = buf[0]) >= 0) && msg->type < MSG_NBR &&
         get_message_size(msg->type, &message_size) && size == message_size) {
      ret = true;
      switch(msg->type) {
         case MSG_OK:
         case MSG_QUIT:
         case MSG_ERROR:
         case MSG_ABORT:
         case MSG_DONE:
         case MSG_GET_VERSION:
            break;
         case MSG_STARTUP:
            for (int i = 0; i < STARTUP_MSG_LEN; ++i) {
               msg->data.startup.message[i] = buf[i+1];
            }
            break;
         case MSG_VERSION:
            msg->data.version.major = buf[1];
            msg->data.version.minor = buf[2];
            msg->data.version.patch = buf[3];
            break;
         case MSG_SET_COMPUTE: 
            memcpy(&(msg->data.set_compute.c_re), &(buf[1 + 0 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.set_compute.c_im), &(buf[1 + 1 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.set_compute.d_re), &(buf[1 + 2 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.set_compute.d_im), &(buf[1 + 3 * sizeof(double)]), sizeof(double));
            msg->data.set_compute.n = buf[1 + 4 * sizeof(double)];
            break;
         case MSG_COMPUTE: // type + chunk_id + nbr_tasks
            msg->data.compute.cid = buf[1];
            memcpy(&(msg->data.compute.re), &(buf[2 + 0 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.compute.im), &(buf[2 + 1 * sizeof(double)]), sizeof(double));
            msg->data.compute.n_re = buf[2 + 2 * sizeof(double) + 0];
            msg->data.compute.n_im = buf[2 + 2 * sizeof(double) + 1];
            break;
         case MSG_COMPUTE_DATA:  // type + chunk_id + task_id + result
            msg->data.compute_data.cid = buf[1];
            msg->data.compute_data.i_re = buf[2];
            msg->data.compute_data.i_im = buf[3];
            msg->data.compute_data.iter = buf[4];
            break;
         case MSG_COMP_BURST:
            msg->data.compute_data.cid = buf[1];
            memcpy(&(msg->data.comp_BURST.re), &(buf[2 + 0 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.comp_BURST.im), &(buf[2 + 1 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.comp_BURST.length), &(buf[2 + 2 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.comp_BURST.iters), &(buf[2 + 3 * sizeof(double)]), sizeof(msg->data.comp_BURST.iters));
            memcpy(&(msg->data.comp_BURST.grid_w), &(buf[2 + 4 * sizeof(double)]), sizeof(double));
            break;
         case MSG_SET_COMP_BURST:
            memcpy(&(msg->data.set_comp_BURST.length), &(buf[1 + 0 * sizeof(double)]), sizeof(double));
            break;
         case MSG_COMPUTE_DATA_BURST:
            msg->data.compute_data_BURST.iters = (uint8_t*) malloc(get_size_iters() * sizeof(uint8_t));
            if (!msg->data.compute_data_BURST.iters) {
               error("allocate for msg fails");
            }         
            memcpy(msg->data.compute_data_BURST.iters, buf, get_size_iters() * sizeof(uint8_t));
            break;
         default: // unknown message type
            ret = false;
            break;
      } // end switch
   }
   return ret;
}

/* end of messages.c */
