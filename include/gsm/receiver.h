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


#ifndef INCLUDED_GSM_RECEIVER_H
#define INCLUDED_GSM_RECEIVER_H

#include <gsm/api.h>
#include <gnuradio/block.h>
#include <gnuradio/feval.h>

namespace gr {
  namespace gsm {

    /*!
     * \brief <+description of block+>
     * \ingroup gsm
     *
     */
    class GSM_API receiver : virtual public block
    {
     public:
      typedef boost::shared_ptr<receiver> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of gsm::receiver.
       *
       * To avoid accidental use of raw pointers, gsm::receiver's
       * constructor is in a private implementation
       * class. gsm::receiver::make is the public interface for
       * creating new instances.
       */
      static sptr make(feval_dd * tuner, int osr, int arfcn);
    };

  } // namespace gsm
} // namespace gr

#endif /* INCLUDED_GSM_RECEIVER_H */

