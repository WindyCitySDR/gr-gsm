/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GSM_RECEIVER_IMPL_H
#define INCLUDED_GSM_RECEIVER_IMPL_H

#include <gsm/receiver.h>
#include <gsm_constants.h>
#include <receiver_config.h>
#include <gsmtap.h>

namespace gr {
  namespace gsm {

    typedef std::vector<gr_complex> vector_complex;

    class receiver_impl : public receiver
    {
     private:
        /**@name Configuration of the receiver */
        //@{
        const int d_OSR; ///< oversampling ratio
        const int d_chan_imp_length; ///< channel impulse length
        uint16_t d_arfcn;
        int8_t d_signal_dbm;
        //@}

        gr_complex d_sch_training_seq[N_SYNC_BITS]; ///<encoded training sequence of a SCH burst
        gr_complex d_norm_training_seq[TRAIN_SEQ_NUM][N_TRAIN_BITS]; ///<encoded training sequences of a normal bursts and dummy bursts

        feval_dd *d_tuner; ///<callback to a python object which is used for frequency tunning

        /** Counts samples consumed by the receiver
         *
         * It is used in beetween find_fcch_burst and reach_sch_burst calls.
         * My intention was to synchronize this counter with some internal sample
         * counter of the USRP. Simple access to such USRP's counter isn't possible
         * so this variable isn't used in the "synchronized" state of the receiver yet.
         */
        unsigned d_counter;

        /**@name Variables used to store result of the find_fcch_burst fuction */
        //@{
        unsigned d_fcch_start_pos; ///< position of the first sample of the fcch burst
        float d_freq_offset; ///< frequency offset of the received signal
        //@}
        std::list<double> d_freq_offset_vals;

        /**@name Identifiers of the BTS extracted from the SCH burst */
        //@{
        int d_ncc; ///< network color code
        int d_bcc; ///< base station color code
        //@}

        /**@name Internal state of the gsm receiver */
        //@{
        enum states {
          first_fcch_search, next_fcch_search, sch_search, // synchronization search part
          synchronized // receiver is synchronized in this state
        } d_state;
        //@}

        /**@name Variables which make internal state in the "synchronized" state */
        //@{
        burst_counter d_burst_nr; ///< frame number and timeslot number
        channel_configuration d_channel_conf; ///< mapping of burst_counter to burst_type
        //@}
    
        unsigned d_failed_sch; ///< number of subsequent erroneous SCH bursts    
        
        /** Function whis is used to search a FCCH burst and to compute frequency offset before
        * "synchronized" state of the receiver
        *
        * TODO: Describe the FCCH search algorithm in the documentation
        * @param input vector with input signal
        * @param nitems number of samples in the input vector
        * @return
        */
        bool find_fcch_burst(const gr_complex *input, const int nitems);

        /** Computes frequency offset from FCCH burst samples
         *
         * @param input vector with input samples
         * @param first_sample number of the first sample of the FCCH busrt
         * @param last_sample number of the last sample of the FCCH busrt
         * @return frequency offset
         */
        double compute_freq_offset(const gr_complex * input, unsigned first_sample, unsigned last_sample);

        /** Calls d_tuner's method to set frequency offset from Python level
         *
         * @param freq_offset absolute frequency offset of the received signal
         */
        void set_frequency(double freq_offset);

        /** Computes angle between two complex numbers
         *
         * @param val1 first complex number
         * @param val2 second complex number
         * @return
         */
        inline float compute_phase_diff(gr_complex val1, gr_complex val2);

        /** Function whis is used to get near to SCH burst
         *
         * @param nitems number of samples in the gsm_receiver's buffer
         * @return true if SCH burst is near, false otherwise
         */
        bool reach_sch_burst(const int nitems);

        /** Extracts channel impulse response from a SCH burst and computes first sample number of this burst
         *
         * @param input vector with input samples
         * @param chan_imp_resp complex vector where channel impulse response will be stored
         * @return number of first sample of the burst
         */
        int get_sch_chan_imp_resp(const gr_complex *input, gr_complex * chan_imp_resp);

        /** MLSE detection of a burst bits
         *
         * Detects bits of burst using viterbi algorithm.
         * @param input vector with input samples
         * @param chan_imp_resp vector with the channel impulse response
         * @param burst_start number of the first sample of the burst
         * @param output_binary vector with output bits
         */
        void detect_burst(const gr_complex * input, gr_complex * chan_imp_resp, int burst_start, unsigned char * output_binary);

        /** Encodes differentially input bits and maps them into MSK states
         *
         * @param input vector with input bits
         * @param nitems number of samples in the "input" vector
         * @param gmsk_output bits mapped into MSK states
         * @param start_point first state
         */
        void gmsk_mapper(const unsigned char * input, int nitems, gr_complex * gmsk_output, gr_complex start_point);

        /** Correlates MSK mapped sequence with input signal
         *
         * @param sequence MKS mapped sequence
         * @param length length of the sequence
         * @param input_signal vector with input samples
         * @return correlation value
         */
        gr_complex correlate_sequence(const gr_complex * sequence, int length, const gr_complex * input);

        /** Computes autocorrelation of input vector for positive arguments
         *
         * @param input vector with input samples
         * @param out output vector
         * @param nitems length of the input vector
         */
        inline void autocorrelation(const gr_complex * input, gr_complex * out, int nitems);

        /** Filters input signal through channel impulse response
         *
         * @param input vector with input samples
         * @param nitems number of samples to pass through filter
         * @param filter filter taps - channel impulse response
         * @param filter_length nember of filter taps
         * @param output vector with filtered samples
         */
        inline void mafi(const gr_complex * input, int nitems, gr_complex * filter, int filter_length, gr_complex * output);

        /**  Extracts channel impulse response from a normal burst and computes first sample number of this burst
         *
         * @param input vector with input samples
         * @param chan_imp_resp complex vector where channel impulse response will be stored
         * @param search_range possible absolute offset of a channel impulse response start
         * @param bcc base station color code - number of a training sequence
         * @return first sample number of normal burst
         */
        int get_norm_chan_imp_resp(const gr_complex *input, gr_complex * chan_imp_resp, float *corr_max, int bcc);

        /**
         *
         */
        void send_burst(burst_counter burst_nr, const unsigned char * burst_binary, burst_type b_type);

        /**
         *
         */
        void configure_receiver();
            
     public:
      receiver_impl(feval_dd * tuner, int osr, int arfcn);
      ~receiver_impl();
      
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      // Where all the action really happens
      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };
  } // namespace gsm
} // namespace gr

#endif /* INCLUDED_GSM_RECEIVER_IMPL_H */

