/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include "config.h"
# include  "StringHeap.h"
# include  "t-dll.h"
# include  "discipline.h"
# include  <cstdlib>
# include  <cstdio>
# include  <cstring>

static StringHeap api_strings;

/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" ivl_island_t ivl_branch_island(ivl_branch_t net)
{
      return net->island;
}

extern "C" ivl_nexus_t ivl_branch_terminal(ivl_branch_t net, int idx)
{
      assert(idx >= 0);
      assert( idx < 2);
      return net->pins[idx];
}
extern "C" const char*ivl_design_delay_sel(ivl_design_t des)
{
      return des->self->get_delay_sel();
}

extern "C" const char*ivl_design_flag(ivl_design_t des, const char*key)
{
      return des->self->get_flag(key);
}

extern "C" int ivl_design_process(ivl_design_t des,
				  ivl_process_f func,
				  void*cd)
{
      for (ivl_process_t idx = des->threads_;  idx;  idx = idx->next_) {
	    int rc = (func)(idx, cd);
	    if (rc != 0)
		  return rc;
      }

      return 0;
}

extern "C" ivl_scope_t ivl_design_root(ivl_design_t des)
{
      cerr << "ANACHRONISM: ivl_design_root called. "
	    "Use ivl_design_roots instead." << endl;

      assert (des->nroots_);
      return des->roots_[0];
}

extern "C" void ivl_design_roots(ivl_design_t des, ivl_scope_t **scopes,
				 unsigned int *nscopes)
{
      assert (nscopes && scopes);
      *scopes = &des->roots_[0];
      *nscopes = des->nroots_;
}

extern "C" int ivl_design_time_precision(ivl_design_t des)
{
      return des->time_precision;
}

extern "C" unsigned ivl_design_consts(ivl_design_t des)
{
      return des->consts.size();
}

extern "C" ivl_net_const_t ivl_design_const(ivl_design_t des, unsigned idx)
{
      assert(idx < des->consts.size());
      return des->consts[idx];
}

extern "C" unsigned ivl_design_disciplines(ivl_design_t des)
{
      assert(des);
      return des->disciplines.size();
}

extern "C" ivl_discipline_t ivl_design_discipline(ivl_design_t des, unsigned idx)
{
      assert(des);
      assert(idx < des->disciplines.size());
      return des->disciplines[idx];
}

extern "C" ivl_dis_domain_t ivl_discipline_domain(ivl_discipline_t net)
{
      return net->domain();
}

extern "C" ivl_nature_t ivl_discipline_flow(ivl_discipline_t net)
{
      return net->flow();
}

extern "C" const char* ivl_discipline_name(ivl_discipline_t net)
{
      return net->name();
}

extern "C" ivl_nature_t ivl_discipline_potential(ivl_discipline_t net)
{
      return net->potential();
}

extern "C" ivl_expr_type_t ivl_expr_type(ivl_expr_t net)
{
      if (net == 0)
	    return IVL_EX_NONE;
      return net->type_;
}

extern "C" const char*ivl_expr_file(ivl_expr_t net)
{
      assert(net);
      return net->file.str();
}

extern "C" unsigned ivl_expr_lineno(ivl_expr_t net)
{
      assert(net);
      return net->lineno;
}

inline static const char *basename(ivl_scope_t scope, const char *inst)
{
      inst += strlen(ivl_scope_name(scope));
      assert(*inst == '.');
      return inst+1;
}

extern "C" ivl_variable_type_t ivl_const_type(ivl_net_const_t net)
{
      assert(net);
      return net->type;
}

extern "C" const char*ivl_const_bits(ivl_net_const_t net)
{
      assert(net);
      switch (net->type) {

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    if (net->width_ <= sizeof(net->b.bit_))
		  return net->b.bit_;
	    else
		  return net->b.bits_;

	  default:
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_const_delay(ivl_net_const_t net, unsigned transition)
{
      assert(transition < 3);
      return net->delay[transition];
}

extern "C" ivl_nexus_t ivl_const_nex(ivl_net_const_t net)
{
      assert(net);
      return net->pin_;
}

extern "C" double ivl_const_real(ivl_net_const_t net)
{
      assert(net);
      assert(net->type == IVL_VT_REAL);
      return net->b.real_value;
}

extern "C" int ivl_const_signed(ivl_net_const_t net)
{
      assert(net);
      return net->signed_;
}

extern "C" unsigned ivl_const_width(ivl_net_const_t net)
{
      assert(net);
      return net->width_;
}

extern "C" const char* ivl_event_name(ivl_event_t net)
{
      static char*name_buffer = 0;
      static unsigned name_size = 0;

      ivl_scope_t scope = net->scope;
      const char*sn = ivl_scope_name(scope);

      unsigned need = strlen(sn) + 1 + strlen(net->name) + 1;
      if (need > name_size) {
	    name_buffer = (char*)realloc(name_buffer, need);
	    name_size = need;
      }

      strcpy(name_buffer, sn);
      char*tmp = name_buffer + strlen(sn);
      *tmp++ = '.';
      strcpy(tmp, net->name);

      cerr << "ANACHRONISM: Call to anachronistic ivl_event_name." << endl;

      return name_buffer;
}

extern "C" const char* ivl_event_basename(ivl_event_t net)
{
      return net->name;
}

extern "C" ivl_scope_t ivl_event_scope(ivl_event_t net)
{
      return net->scope;
}

extern "C" unsigned ivl_event_nany(ivl_event_t net)
{
      assert(net);
      return net->nany;
}

extern "C" ivl_nexus_t ivl_event_any(ivl_event_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nany);
      return net->pins[idx];
}

extern "C" unsigned ivl_event_nneg(ivl_event_t net)
{
      assert(net);
      return net->nneg;
}

extern "C" ivl_nexus_t ivl_event_neg(ivl_event_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nneg);
      return net->pins[net->nany + idx];
}

extern "C" unsigned ivl_event_npos(ivl_event_t net)
{
      assert(net);
      return net->npos;
}

extern "C" ivl_nexus_t ivl_event_pos(ivl_event_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->npos);
      return net->pins[net->nany + net->nneg + idx];
}

extern "C" const char* ivl_expr_bits(ivl_expr_t net)
{
      assert(net && (net->type_ == IVL_EX_NUMBER));
      return net->u_.number_.bits_;
}

extern "C" ivl_branch_t ivl_expr_branch(ivl_expr_t net)
{
      assert(net && (net->type_ == IVL_EX_BACCESS));
      return net->u_.branch_.branch;
}

extern "C" ivl_scope_t ivl_expr_def(ivl_expr_t net)
{
      assert(net);

      switch (net->type_) {

	  case IVL_EX_UFUNC:
	    return net->u_.ufunc_.def;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" uint64_t ivl_expr_delay_val(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_DELAY);
      return net->u_.delay_.value;
}

extern "C" double ivl_expr_dvalue(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_REALNUM);
      return net->u_.real_.value;
}

extern "C" const char* ivl_expr_name(ivl_expr_t net)
{
      switch (net->type_) {

	  case IVL_EX_SFUNC:
	    return net->u_.sfunc_.name_;

	  case IVL_EX_SIGNAL:
	    return net->u_.signal_.sig->name_;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_nature_t ivl_expr_nature(ivl_expr_t net)
{
      assert(net && (net->type_ == IVL_EX_BACCESS));
      return net->u_.branch_.nature;
}

extern "C" char ivl_expr_opcode(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BINARY:
	    return net->u_.binary_.op_;

	  case IVL_EX_UNARY:
	    return net->u_.unary_.op_;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_expr_t ivl_expr_oper1(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BINARY:
	  case IVL_EX_SELECT:
	    return net->u_.binary_.lef_;

	  case IVL_EX_UNARY:
	    return net->u_.unary_.sub_;

	  case IVL_EX_MEMORY:
	    return net->u_.memory_.idx_;

	  case IVL_EX_SIGNAL:
	    return net->u_.signal_.word;

	  case IVL_EX_TERNARY:
	    return net->u_.ternary_.cond;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_expr_t ivl_expr_oper2(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BINARY:
	  case IVL_EX_SELECT:
	    return net->u_.binary_.rig_;

	  case IVL_EX_TERNARY:
	    return net->u_.ternary_.true_e;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_expr_t ivl_expr_oper3(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_TERNARY:
	    return net->u_.ternary_.false_e;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_parameter_t ivl_expr_parameter(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_NUMBER:
	    return net->u_.number_.parameter;
	  case IVL_EX_STRING:
	    return net->u_.string_.parameter;
	  case IVL_EX_REALNUM:
	    return net->u_.real_.parameter;
	  default:
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_expr_parm(ivl_expr_t net, unsigned idx)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_CONCAT:
	    assert(idx < net->u_.concat_.parms);
	    return net->u_.concat_.parm[idx];

	  case IVL_EX_SFUNC:
	    assert(idx < net->u_.sfunc_.parms);
	    return net->u_.sfunc_.parm[idx];

	  case IVL_EX_UFUNC:
	    assert(idx < net->u_.ufunc_.parms);
	    return net->u_.ufunc_.parm[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_expr_parms(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_CONCAT:
	    return net->u_.concat_.parms;

	  case IVL_EX_SFUNC:
	    return net->u_.sfunc_.parms;

	  case IVL_EX_UFUNC:
	    return net->u_.ufunc_.parms;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_expr_repeat(ivl_expr_t net)
{
      assert(net);
      assert(net->type_ == IVL_EX_CONCAT);
      return net->u_.concat_.rept;
}

extern "C" ivl_event_t ivl_expr_event(ivl_expr_t net)
{
      assert(net);
      assert(net->type_ == IVL_EX_EVENT);
      return net->u_.event_.event;
}

extern "C" ivl_scope_t ivl_expr_scope(ivl_expr_t net)
{
      assert(net);
      assert(net->type_ == IVL_EX_SCOPE);
      return net->u_.scope_.scope;
}

extern "C" ivl_signal_t ivl_expr_signal(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_SIGNAL:
	  case IVL_EX_ARRAY:
	    return net->u_.signal_.sig;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" int ivl_expr_signed(ivl_expr_t net)
{
      assert(net);
      return net->signed_;
}

extern "C" int ivl_expr_sized(ivl_expr_t net)
{
      assert(net);
      return net->sized_;
}

extern "C" const char* ivl_expr_string(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_STRING);
      return net->u_.string_.value_;
}

extern "C" unsigned long ivl_expr_uvalue(ivl_expr_t net)
{
      switch (net->type_) {

	  case IVL_EX_ULONG:
	    return net->u_.ulong_.value;

	  case IVL_EX_NUMBER: {
		unsigned long val = 0;
		for (unsigned long idx = 0 ;  idx < net->width_ ;  idx += 1) {
		      if (net->u_.number_.bits_[idx] == '1')
			    val |= 1UL << idx;
		}

		return val;
	  }

	  default:
	    assert(0);
      }

      assert(0);
      return 0;
}

extern "C" ivl_variable_type_t ivl_expr_value(ivl_expr_t net)
{
      assert(net);
      return net->value_;
}

extern "C" unsigned ivl_expr_width(ivl_expr_t net)
{
      assert(net);
      return net->width_;
}

/*
 *  ivl_file_table_index puts entries in the map as needed and returns
 *  the appropriate index.
 *  ivl_file_table_size returns the number of entries in the table.
 *  ivl_file_table_item returns the file name for the given index.
 */
struct ltstr
{
      bool operator()(const char*s1, const char*s2) const
      {
	    return strcmp(s1, s2) < 0;
      }
};
static map<const char*, unsigned, ltstr> fn_map;
static vector<const char*> fn_vector;

static void ivl_file_table_init()
{
        /* The first two index entries do not depend on a real
         * file name and are always available. */
      fn_vector.push_back("N/A");
      fn_map["N/A"] = 0;
      fn_vector.push_back("<interactive>");
      fn_map["<interactive>"] = 1;
}

extern "C" const char* ivl_file_table_item(unsigned idx)
{
      if (fn_vector.empty()) {
	    ivl_file_table_init();
      }

      assert(idx < fn_vector.size());
      return fn_vector[idx];
}

extern "C" unsigned ivl_file_table_index(const char*name)
{
      if (fn_vector.empty()) {
	    ivl_file_table_init();
      }

      if (name == NULL) return 0;

        /* The new index is the current map size. This is inserted only
         * if the file name is not currently in the map. */
      pair<map<const char*, unsigned, ltstr>::iterator, bool> result;
      result = fn_map.insert(make_pair(name, fn_vector.size()));
      if (result.second) {
	    fn_vector.push_back(name);
      }
      return result.first->second;
}

extern "C" unsigned ivl_file_table_size()
{
      if (fn_vector.empty()) {
	    ivl_file_table_init();
      }

      return fn_vector.size();
}

extern "C" int ivl_island_flag_set(ivl_island_t net, unsigned flag, int value)
{
      if (flag >= net->flags.size()) {
	    if (value == 0)
		  return 0;
	    else
		  net->flags.resize(flag+1, false);
      }

      int old_flag = net->flags[flag];
      net->flags[flag] = value != 0;
      return old_flag;
}

extern "C" int ivl_island_flag_test(ivl_island_t net, unsigned flag)
{
      if (flag >= net->flags.size())
	    return 0;
      else
	    return net->flags[flag];
}

extern "C" const char* ivl_logic_attr(ivl_net_logic_t net, const char*key)
{
      assert(net);
      unsigned idx;

      for (idx = 0 ;  idx < net->nattr ;  idx += 1) {

	    if (strcmp(net->attr[idx].key, key) == 0)
		  return net->attr[idx].type == IVL_ATT_STR
			? net->attr[idx].val.str
			: 0;
      }

      return 0;
}

extern "C" unsigned ivl_logic_attr_cnt(ivl_net_logic_t net)
{
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_logic_attr_val(ivl_net_logic_t net,
					      unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" ivl_drive_t ivl_logic_drive0(ivl_net_logic_t net)
{
      ivl_nexus_t nex = ivl_logic_pin(net, 0);

      for (unsigned idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t cur = ivl_nexus_ptr(nex, idx);
	    if (ivl_nexus_ptr_log(cur) != net)
		  continue;
	    if (ivl_nexus_ptr_pin(cur) != 0)
		  continue;
	    return ivl_nexus_ptr_drive0(cur);
      }

      assert(0);
      return IVL_DR_STRONG;
}

extern "C" ivl_drive_t ivl_logic_drive1(ivl_net_logic_t net)
{
      ivl_nexus_t nex = ivl_logic_pin(net, 0);

      for (unsigned idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t cur = ivl_nexus_ptr(nex, idx);
	    if (ivl_nexus_ptr_log(cur) != net)
		  continue;
	    if (ivl_nexus_ptr_pin(cur) != 0)
		  continue;
	    return ivl_nexus_ptr_drive1(cur);
      }

      assert(0);
      return IVL_DR_STRONG;
}

extern "C" const char* ivl_logic_name(ivl_net_logic_t net)
{
      assert(net);
      cerr << "ANACHRONISM: Call to anachronistic ivl_logic_name." << endl;
      return net->name_;
}

extern "C" const char* ivl_logic_basename(ivl_net_logic_t net)
{
      assert(net);
      return net->name_;
}

extern "C" ivl_scope_t ivl_logic_scope(ivl_net_logic_t net)
{
      assert(net);
      return net->scope_;
}

extern "C" ivl_logic_t ivl_logic_type(ivl_net_logic_t net)
{
      return net->type_;
}

extern "C" unsigned ivl_logic_pins(ivl_net_logic_t net)
{
      return net->npins_;
}

extern "C" ivl_nexus_t ivl_logic_pin(ivl_net_logic_t net, unsigned pin)
{
      assert(pin < net->npins_);
      return net->pins_[pin];
}

extern "C" ivl_udp_t ivl_logic_udp(ivl_net_logic_t net)
{
      assert(net->type_ == IVL_LO_UDP);
      assert(net->udp);
      return net->udp;
}

extern "C" ivl_expr_t ivl_logic_delay(ivl_net_logic_t net, unsigned transition)
{
      assert(transition < 3);
      return net->delay[transition];
}

extern "C" unsigned ivl_logic_width(ivl_net_logic_t net)
{
      assert(net);
      return net->width_;
}

extern "C" int  ivl_udp_sequ(ivl_udp_t net)
{
      return net->sequ;
}

extern "C" unsigned    ivl_udp_nin(ivl_udp_t net)
{
      return net->nin;
}

extern "C" unsigned    ivl_udp_init(ivl_udp_t net)
{
      return net->init;
}

extern "C" const char* ivl_udp_row(ivl_udp_t net, unsigned idx)
{
      assert(idx < net->nrows);
      assert(net->table);
      assert(net->table[idx]);
      return net->table[idx];
}

extern "C" unsigned    ivl_udp_rows(ivl_udp_t net)
{
      return net->nrows;
}

extern "C" const char* ivl_udp_name(ivl_udp_t net)
{
      assert(net->name);
      return net->name;
}

extern "C" const char* ivl_lpm_basename(ivl_lpm_t net)
{
      return net->name;
}

extern "C" ivl_nexus_t ivl_lpm_async_clr(ivl_lpm_t net)
{
      assert(net);
      switch(net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.aclr;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_sync_clr(ivl_lpm_t net)
{
      assert(net);
      switch(net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.sclr;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_lpm_delay(ivl_lpm_t net, unsigned transition)
{
      assert(transition < 3);
      return net->delay[transition];
}

extern "C" ivl_nexus_t ivl_lpm_async_set(ivl_lpm_t net)
{
      assert(net);
      switch(net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.aset;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_sync_set(ivl_lpm_t net)
{
      assert(net);
      switch(net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.sset;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_signal_t ivl_lpm_array(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_ARRAY:
	    return net->u_.array.sig;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_lpm_base(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV:
	    return net->u_.part.base;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_clk(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.clk;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_lpm_aset_value(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.aset_value;
	  default:
	    assert(0);
	    return 0;
      }
}
extern "C" ivl_expr_t ivl_lpm_sset_value(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.sset_value;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_scope_t ivl_lpm_define(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.def;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_enable(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	    return net->u_.ff.we;
	  default:
	    assert(0);
	    return 0;
      }
}

/* The file name and line number are only set for system functions! */
extern "C" const char* ivl_lpm_file(ivl_lpm_t net)
{
      return net->file.str();
}

extern "C" unsigned ivl_lpm_lineno(ivl_lpm_t net)
{
      return net->lineno;
}

extern "C" ivl_nexus_t ivl_lpm_data(ivl_lpm_t net, unsigned idx)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_ABS:
	  case IVL_LPM_CAST_INT:
	  case IVL_LPM_CAST_REAL:
	    assert(idx == 0);
	    return net->u_.arith.a;

	  case IVL_LPM_ADD:
	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_CMP_NEE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_POW:
	  case IVL_LPM_SUB:
	    assert(idx <= 1);
	    if (idx == 0)
		  return net->u_.arith.a;
	    else
		  return net->u_.arith.b;

	  case IVL_LPM_MUX:
	    assert(idx < net->u_.mux.size);
	    return net->u_.mux.d[idx];

	  case IVL_LPM_RE_AND:
	  case IVL_LPM_RE_OR:
	  case IVL_LPM_RE_XOR:
	  case IVL_LPM_RE_NAND:
	  case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_XNOR:
	  case IVL_LPM_SIGN_EXT:
	    assert(idx == 0);
	    return net->u_.reduce.a;

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    assert(idx <= 1);
	    if (idx == 0)
		  return net->u_.shift.d;
	    else
		  return net->u_.shift.s;

	  case IVL_LPM_FF:
	    assert(idx == 0);
	    return net->u_.ff.d.pin;

	  case IVL_LPM_CONCAT:
	    assert(idx < net->u_.concat.inputs);
	    return net->u_.concat.pins[idx+1];

	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV:
	    assert(idx <= 1);
	    if (idx == 0)
		  return net->u_.part.a;
	    else
		  return net->u_.part.s;

	  case IVL_LPM_REPEAT:
	    assert(idx == 0);
	    return net->u_.repeat.a;

	  case IVL_LPM_SFUNC:
	      // Skip the return port.
	    assert(idx < (net->u_.sfunc.ports-1));
	    return net->u_.sfunc.pins[idx+1];

	  case IVL_LPM_UFUNC:
	      // Skip the return port.
	    assert(idx < (net->u_.ufunc.ports-1));
	    return net->u_.ufunc.pins[idx+1];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_datab(ivl_lpm_t net, unsigned idx)
{
      cerr << "ANACHRONISM: Call to anachronistic ivl_lpm_datab." << endl;
      assert(net);
      switch (net->type) {

	  case IVL_LPM_ADD:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_POW:
	  case IVL_LPM_SUB:
	    assert(idx == 0);
	    return net->u_.arith.b;

	  default:
	    assert(0);
	    return 0;
      }
}


/*
 * This function returns the hierarchical name for the LPM device. The
 * name needs to be built up from the scope name and the lpm base
 * name.
 *
 * Anachronism: This function is provided for
 * compatibility. Eventually, it will be removed.
 */
extern "C" const char* ivl_lpm_name(ivl_lpm_t net)
{
      static char*name_buffer = 0;
      static unsigned name_size = 0;

      ivl_scope_t scope = ivl_lpm_scope(net);
      const char*sn = ivl_scope_name(scope);

      unsigned need = strlen(sn) + 1 + strlen(net->name) + 1;
      if (need > name_size) {
	    name_buffer = (char*)realloc(name_buffer, need);
	    name_size = need;
      }

      strcpy(name_buffer, sn);
      char*tmp = name_buffer + strlen(sn);
      *tmp++ = '.';
      strcpy(tmp, net->name);
      return name_buffer;
}


extern "C" ivl_nexus_t ivl_lpm_q(ivl_lpm_t net)
{
      assert(net);

      switch (net->type) {
	  case IVL_LPM_ABS:
	  case IVL_LPM_ADD:
	  case IVL_LPM_CAST_INT:
	  case IVL_LPM_CAST_REAL:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_NEE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_POW:
	  case IVL_LPM_SUB:
	    return net->u_.arith.q;

	  case IVL_LPM_FF:
	    return net->u_.ff.q.pin;

	  case IVL_LPM_MUX:
	    return net->u_.mux.q;

	  case IVL_LPM_RE_AND:
	  case IVL_LPM_RE_OR:
	  case IVL_LPM_RE_XOR:
	  case IVL_LPM_RE_NAND:
	  case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_XNOR:
	  case IVL_LPM_SIGN_EXT:
	    return net->u_.reduce.q;

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    return net->u_.shift.q;

	  case IVL_LPM_SFUNC:
	    return net->u_.sfunc.pins[0];

	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.pins[0];

	  case IVL_LPM_CONCAT:
	    return net->u_.concat.pins[0];

	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV:
	    return net->u_.part.q;

	  case IVL_LPM_REPEAT:
	    return net->u_.repeat.q;

	  case IVL_LPM_ARRAY:
	    return net->u_.array.q;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_drive_t ivl_lpm_drive0(ivl_lpm_t net)
{
      ivl_nexus_t nex = ivl_lpm_q(net);

      for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex) ; idx += 1) {
	    ivl_nexus_ptr_t cur = ivl_nexus_ptr(nex, idx);
	    if (ivl_nexus_ptr_lpm(cur) != net)
		  continue;
	    if (ivl_nexus_ptr_pin(cur) != 0)
		  continue;
	    return ivl_nexus_ptr_drive0(cur);
      }

      assert(0);
      return IVL_DR_STRONG;
}

extern "C" ivl_drive_t ivl_lpm_drive1(ivl_lpm_t net)
{
      ivl_nexus_t nex = ivl_lpm_q(net);

      for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex) ; idx += 1) {
	    ivl_nexus_ptr_t cur = ivl_nexus_ptr(nex, idx);
	    if (ivl_nexus_ptr_lpm(cur) != net)
		  continue;
	    if (ivl_nexus_ptr_pin(cur) != 0)
		  continue;
	    return ivl_nexus_ptr_drive1(cur);
      }

      assert(0);
      return IVL_DR_STRONG;
}

extern "C" ivl_scope_t ivl_lpm_scope(ivl_lpm_t net)
{
      assert(net);
      return net->scope;
}

extern "C" ivl_nexus_t ivl_lpm_select(ivl_lpm_t net)
{
      switch (net->type) {

	  case IVL_LPM_MUX:
	    return net->u_.mux.s;

	  case IVL_LPM_ARRAY:
	    return net->u_.array.a;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_lpm_selects(ivl_lpm_t net)
{
      switch (net->type) {
	  case IVL_LPM_MUX:
	    return net->u_.mux.swid;
	  case IVL_LPM_ARRAY:
	    return net->u_.array.swid;
	  case IVL_LPM_CONCAT:
	    cerr << "error: ivl_lpm_selects() is no longer supported for "
	            "IVL_LPM_CONCAT, use ivl_lpm_size() instead." << endl;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" int ivl_lpm_signed(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	  case IVL_LPM_MUX:
	    return 0;
	  case IVL_LPM_ABS:
	  case IVL_LPM_ADD:
	  case IVL_LPM_CAST_REAL:
	  case IVL_LPM_CMP_EEQ:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_CMP_NEE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_POW:
	  case IVL_LPM_SUB:
	    return net->u_.arith.signed_flag;
	  case IVL_LPM_RE_AND:
	  case IVL_LPM_RE_OR:
	  case IVL_LPM_RE_XOR:
	  case IVL_LPM_RE_NAND:
	  case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_XNOR:
	    return 0;
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    return net->u_.shift.signed_flag;
	  case IVL_LPM_CAST_INT:
	  case IVL_LPM_SIGN_EXT: // Sign extend is always signed.
	    return 1;
	  case IVL_LPM_SFUNC:
	    return 0;
	  case IVL_LPM_UFUNC:
	    return 0;
	  case IVL_LPM_CONCAT: // Concatenations are always unsigned
	    return 0;
	  case IVL_LPM_PART_VP:
	  case IVL_LPM_PART_PV:
	    return net->u_.part.signed_flag;
	  case IVL_LPM_REPEAT:
	    return 0;
	  case IVL_LPM_ARRAY: // Array ports take the signedness of the array.
	    return net->u_.array.sig->signed_;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_lpm_size(ivl_lpm_t net)
{
      switch (net->type) {
	  case IVL_LPM_MUX:
	    return net->u_.mux.size;
	  case IVL_LPM_SFUNC:
	    return net->u_.sfunc.ports - 1;
	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.ports - 1;
	  case IVL_LPM_REPEAT:
	    return net->u_.repeat.count;
	  case IVL_LPM_CONCAT:
	    return net->u_.concat.inputs;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" const char* ivl_lpm_string(ivl_lpm_t net)
{
      assert(net->type == IVL_LPM_SFUNC);
      return net->u_.sfunc.fun_name;
}

extern "C" ivl_lpm_type_t ivl_lpm_type(ivl_lpm_t net)
{
      return net->type;
}

extern "C" unsigned ivl_lpm_width(ivl_lpm_t net)
{
      assert(net);
      return net->width;
}

extern "C" ivl_event_t ivl_lpm_trigger(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_SFUNC:
	    return net->u_.sfunc.trigger;
	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.trigger;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_lval_mux(ivl_lval_t net)
{
      assert(net);
      if (net->type_ == IVL_LVAL_MUX)
	    return net->idx;
      return 0x0;
}

extern "C" ivl_expr_t ivl_lval_idx(ivl_lval_t net)
{
      assert(net);

      if (net->type_ == IVL_LVAL_ARR)
	    return net->idx;
      return 0x0;
}

extern "C" ivl_expr_t ivl_lval_part_off(ivl_lval_t net)
{
      assert(net);
      return net->loff;
}

extern "C" unsigned ivl_lval_width(ivl_lval_t net)
{
      assert(net);
      return net->width_;
}

extern "C" ivl_signal_t ivl_lval_sig(ivl_lval_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_LVAL_REG:
	  case IVL_LVAL_NET:
	  case IVL_LVAL_MUX:
	  case IVL_LVAL_ARR:
	    return net->n.sig;
	  default:
	    return 0;
      }
}

extern "C" const char* ivl_nature_name(ivl_nature_t net)
{
      return net->name();
}

/*
 * The nexus name is rarely needed. (Shouldn't be needed at all!) This
 * function will calculate the name if it is not already calculated.
 */
extern "C" const char* ivl_nexus_name(ivl_nexus_t net)
{
      assert(net);
      if (net->name_ == 0) {
	    char tmp[2 * sizeof(net) + 5];
	    snprintf(tmp, sizeof tmp, "n%p", net);
	    net->name_ = api_strings.add(tmp);
      }
      return net->name_;
}

extern "C" void* ivl_nexus_get_private(ivl_nexus_t net)
{
      assert(net);
      return net->private_data;
}

extern "C" void ivl_nexus_set_private(ivl_nexus_t net, void*data)
{
      assert(net);
      net->private_data = data;
}

extern "C" unsigned ivl_nexus_ptrs(ivl_nexus_t net)
{
      assert(net);
      return net->ptrs_.size();
}

extern "C" ivl_nexus_ptr_t ivl_nexus_ptr(ivl_nexus_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->ptrs_.size());
      return & net->ptrs_[idx];
}

extern "C" ivl_drive_t ivl_nexus_ptr_drive0(ivl_nexus_ptr_t net)
{
      assert(net);
      return (ivl_drive_t)(net->drive0);
}

extern "C" ivl_drive_t ivl_nexus_ptr_drive1(ivl_nexus_ptr_t net)
{
      assert(net);
      return (ivl_drive_t)(net->drive1);
}

extern "C" unsigned ivl_nexus_ptr_pin(ivl_nexus_ptr_t net)
{
      assert(net);
      return net->pin_;
}

extern "C" ivl_branch_t ivl_nexus_ptr_branch(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_BRA)
	    return 0;
      return net->l.bra;
}

extern "C" ivl_net_const_t ivl_nexus_ptr_con(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_CON)
	    return 0;
      return net->l.con;
}

extern "C" ivl_net_logic_t ivl_nexus_ptr_log(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_LOG)
	    return 0;
      return net->l.log;
}

extern "C" ivl_lpm_t ivl_nexus_ptr_lpm(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_LPM)
	    return 0;
      return net->l.lpm;
}

extern "C" ivl_signal_t ivl_nexus_ptr_sig(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_SIG)
	    return 0;
      return net->l.sig;
}

extern "C" ivl_switch_t ivl_nexus_ptr_switch(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_SWI)
	    return 0;
      return net->l.swi;
}

extern "C" const char* ivl_parameter_basename(ivl_parameter_t net)
{
      assert(net);
      return net->basename;
}

extern "C" ivl_expr_t ivl_parameter_expr(ivl_parameter_t net)
{
      assert(net);
      return net->value;
}

extern "C" const char* ivl_parameter_file(ivl_parameter_t net)
{
      assert(net);
      return net->file.str();
}

extern "C" unsigned ivl_parameter_lineno(ivl_parameter_t net)
{
      assert(net);
      return net->lineno;
}

extern "C" ivl_scope_t ivl_parameter_scope(ivl_parameter_t net)
{
      assert(net);
      return net->scope;
}

extern "C" ivl_nexus_t ivl_path_condit(ivl_delaypath_t obj)
{
      assert(obj);
      return obj->condit;
}

extern "C" int ivl_path_is_condit(ivl_delaypath_t obj)
{
      assert(obj);
      return obj->conditional ? 1 : 0;
}

extern uint64_t ivl_path_delay(ivl_delaypath_t obj, ivl_path_edge_t edg)
{
      assert(obj);
      return obj->delay[edg];
}

extern ivl_scope_t ivl_path_scope(ivl_delaypath_t obj)
{
      assert(obj);
      assert(obj->scope);
      return obj->scope;
}

extern ivl_nexus_t ivl_path_source(ivl_delaypath_t net)
{
      return net->src;
}

extern int ivl_path_source_posedge(ivl_delaypath_t net)
{
      return net->posedge ? 1 : 0;
}

extern int ivl_path_source_negedge(ivl_delaypath_t net)
{
      return net->negedge ? 1 : 0;
}

extern "C" const char*ivl_process_file(ivl_process_t net)
{
      assert(net);
      return net->file.str();
}

extern "C" unsigned ivl_process_lineno(ivl_process_t net)
{
      assert(net);
      return net->lineno;
}

extern "C" ivl_process_type_t ivl_process_type(ivl_process_t net)
{
      return net->type_;
}

extern "C" int ivl_process_analog(ivl_process_t net)
{
      return net->analog_flag != 0;
}

extern "C" ivl_scope_t ivl_process_scope(ivl_process_t net)
{
      return net->scope_;
}

extern "C" ivl_statement_t ivl_process_stmt(ivl_process_t net)
{
      return net->stmt_;
}

extern "C" unsigned ivl_process_attr_cnt(ivl_process_t net)
{
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_process_attr_val(ivl_process_t net,
						unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" unsigned ivl_scope_attr_cnt(ivl_scope_t net)
{
      assert(net);
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_scope_attr_val(ivl_scope_t net,
					      unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" const char* ivl_scope_basename(ivl_scope_t net)
{
      assert(net);

      return net->name_;
}

extern "C" int ivl_scope_children(ivl_scope_t net,
				  ivl_scope_f func,
				  void*cd)
{
      for (map<hname_t,ivl_scope_t>::iterator cur = net->children.begin()
		 ; cur != net->children.end() ; cur ++) {
	    int rc = func(cur->second, cd);
	    if (rc != 0)
		  return rc;
      }

      return 0;
}

extern "C" ivl_statement_t ivl_scope_def(ivl_scope_t net)
{
      assert(net);
      return net->def;
}

extern "C" const char*ivl_scope_def_file(ivl_scope_t net)
{
      assert(net);
      return net->def_file.str();
}

extern "C" unsigned ivl_scope_def_lineno(ivl_scope_t net)
{
      assert(net);
      return net->def_lineno;
}

extern "C" unsigned ivl_scope_events(ivl_scope_t net)
{
      assert(net);
      return net->nevent_;
}

extern "C" ivl_event_t ivl_scope_event(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nevent_);
      return net->event_[idx];
}

extern "C" const char*ivl_scope_file(ivl_scope_t net)
{
      assert(net);
      return net->file.str();
}

extern "C" unsigned ivl_scope_is_auto(ivl_scope_t net)
{
      assert(net);
      return net->is_auto;
}

extern "C" unsigned ivl_scope_is_cell(ivl_scope_t net)
{
      assert(net);
      return net->is_cell;
}

extern "C" unsigned ivl_scope_lineno(ivl_scope_t net)
{
      assert(net);
      return net->lineno;
}

extern "C" unsigned ivl_scope_logs(ivl_scope_t net)
{
      assert(net);
      return net->nlog_;
}

extern "C" ivl_net_logic_t ivl_scope_log(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nlog_);
      return net->log_[idx];
}

extern "C" unsigned ivl_scope_lpms(ivl_scope_t net)
{
      assert(net);
      return net->nlpm_;
}

extern "C" ivl_lpm_t ivl_scope_lpm(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nlpm_);
      return net->lpm_[idx];
}

static unsigned scope_name_len(ivl_scope_t net)
{
      unsigned len = 0;

      for (ivl_scope_t cur = net ;  cur ;  cur = cur->parent)
	    len += strlen(cur->name_) + 1;

      return len;
}

static void push_scope_basename(ivl_scope_t net, char*buf)
{
      if (net->parent == 0) {
	    strcpy(buf, net->name_);
	    return;
      }

      push_scope_basename(net->parent, buf);
      strcat(buf, ".");
      strcat(buf, net->name_);
}

extern "C" const char* ivl_scope_name(ivl_scope_t net)
{
      static char*name_buffer = 0;
      static unsigned name_size = 0;

      if (net->parent == 0)
	    return net->name_;

      unsigned needlen = scope_name_len(net);

      if (name_size < needlen) {
	    name_buffer = (char*)realloc(name_buffer, needlen);
	    name_size = needlen;
      }


      push_scope_basename(net, name_buffer);

      return name_buffer;
}

extern "C" unsigned ivl_scope_params(ivl_scope_t net)
{
      assert(net);
      return net->nparam_;
}

extern "C" ivl_parameter_t ivl_scope_param(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nparam_);
      return net->param_ + idx;
}

extern "C" ivl_scope_t ivl_scope_parent(ivl_scope_t net)
{
      assert(net);
      return net->parent;
}

extern "C" unsigned ivl_scope_ports(ivl_scope_t net)
{
      assert(net);
      assert(net->type_ == IVL_SCT_FUNCTION);
      return net->ports;
}

extern "C" ivl_signal_t ivl_scope_port(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(net->type_ == IVL_SCT_FUNCTION);
      assert(idx < net->ports);
      return net->port[idx];
}

extern "C" unsigned ivl_scope_sigs(ivl_scope_t net)
{
      assert(net);
      return net->nsigs_;
}

extern "C" ivl_signal_t ivl_scope_sig(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nsigs_);
      return net->sigs_[idx];
}

extern "C" unsigned ivl_scope_switches(ivl_scope_t net)
{
      assert(net);
      return net->switches.size();
}

extern "C" ivl_switch_t ivl_scope_switch(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->switches.size());
      return net->switches[idx];
}

extern "C" int ivl_scope_time_precision(ivl_scope_t net)
{
      assert(net);
      return net->time_precision;
}

extern "C" int ivl_scope_time_units(ivl_scope_t net)
{
      assert(net);
      return net->time_units;
}

extern "C" ivl_scope_type_t ivl_scope_type(ivl_scope_t net)
{
      assert(net);
      return net->type_;
}

extern "C" const char* ivl_scope_tname(ivl_scope_t net)
{
      assert(net);
      return net->tname_;
}

extern "C" int ivl_signal_array_base(ivl_signal_t net)
{
      return net->array_base;
}

extern "C" unsigned ivl_signal_array_count(ivl_signal_t net)
{
      return net->array_words;
}

extern "C" unsigned ivl_signal_array_addr_swapped(ivl_signal_t net)
{
      return net->array_addr_swapped;
}

extern "C" unsigned ivl_signal_dimensions(ivl_signal_t net)
{
      return net->array_dimensions_;
}

extern "C" ivl_discipline_t ivl_signal_discipline(ivl_signal_t net)
{
      return net->discipline;
}

extern "C" const char* ivl_signal_attr(ivl_signal_t net, const char*key)
{
      if (net->nattr == 0)
	    return 0;

      for (unsigned idx = 0 ;  idx < net->nattr ;  idx += 1)

	    if (strcmp(key, net->attr[idx].key) == 0)
		  return net->attr[idx].type == IVL_ATT_STR
			? net->attr[idx].val.str
			: 0;

      return 0;
}

extern "C" unsigned ivl_signal_attr_cnt(ivl_signal_t net)
{
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_signal_attr_val(ivl_signal_t net, unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" const char* ivl_signal_basename(ivl_signal_t net)
{
      return net->name_;
}

extern "C" const char* ivl_signal_name(ivl_signal_t net)
{
      static char*name_buffer = 0;
      static unsigned name_size = 0;

      unsigned needlen = scope_name_len(net->scope_);
      needlen += strlen(net->name_) + 2;

      if (name_size < needlen) {
	    name_buffer = (char*)realloc(name_buffer, needlen);
	    name_size = needlen;
      }

      push_scope_basename(net->scope_, name_buffer);
      strcat(name_buffer, ".");
      strcat(name_buffer, net->name_);

      return name_buffer;
}

extern "C" ivl_nexus_t ivl_signal_nex(ivl_signal_t net, unsigned word)
{
      assert(word < net->array_words);
      if (net->array_words > 1) {
	    if (net->pins) {
		return net->pins[word];
	    } else {
		// net->pins can be NULL for a virtualized reg array.
		assert(net->type_ == IVL_SIT_REG);
		return NULL;
	    }
      } else {
	    return net->pin;
      }
}

extern "C" int ivl_signal_msb(ivl_signal_t net)
{
      assert(net->lsb_dist == 1 || net->lsb_dist == -1);
      return net->lsb_index + net->lsb_dist * (net->width_ - 1);
}

extern "C" int ivl_signal_lsb(ivl_signal_t net)
{
      return net->lsb_index;
}

extern "C" ivl_scope_t ivl_signal_scope(ivl_signal_t net)
{
      assert(net);
      return net->scope_;
}

extern "C" unsigned ivl_signal_width(ivl_signal_t net)
{
      return net->width_;
}

extern "C" ivl_signal_port_t ivl_signal_port(ivl_signal_t net)
{
      return net->port_;
}

extern "C" int ivl_signal_local(ivl_signal_t net)
{
      return net->local_;
}

extern "C" int ivl_signal_signed(ivl_signal_t net)
{
      return net->signed_;
}

extern "C" const char* ivl_signal_file(ivl_signal_t net)
{
      assert(net);
      return net->file.str();
}

extern "C" unsigned ivl_signal_lineno(ivl_signal_t net)
{
      assert(net);
      return net->lineno;
}

extern "C" int ivl_signal_integer(ivl_signal_t net)
{
      return net->isint_;
}

extern "C" ivl_variable_type_t ivl_signal_data_type(ivl_signal_t net)
{
      return net->data_type;
}

extern "C" unsigned ivl_signal_npath(ivl_signal_t net)
{
      return net->npath;
}

extern "C" ivl_delaypath_t ivl_signal_path(ivl_signal_t net, unsigned idx)
{
      assert(idx < net->npath);
      return net->path + idx;
}

extern "C" ivl_signal_type_t ivl_signal_type(ivl_signal_t net)
{
      return net->type_;
}

extern "C" ivl_statement_type_t ivl_statement_type(ivl_statement_t net)
{
      return net->type_;
}

extern "C" const char* ivl_stmt_file(ivl_statement_t net)
{
      return net->file.str();
}

extern "C" unsigned ivl_stmt_lineno(ivl_statement_t net)
{
      return net->lineno;
}

extern "C" ivl_scope_t ivl_stmt_block_scope(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	    return net->u_.block_.scope;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_stmt_block_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	    return net->u_.block_.nstmt_;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net,
					       unsigned i)
{
      switch (net->type_) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	    return net->u_.block_.stmt_ + i;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_scope_t ivl_stmt_call(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ALLOC:
	    return net->u_.alloc_.scope;

	  case IVL_ST_DISABLE:
	    return net->u_.disable_.scope;

	  case IVL_ST_FREE:
	    return net->u_.free_.scope;

	  case IVL_ST_UTASK:
	    return net->u_.utask_.def;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_stmt_case_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_CASE:
	  case IVL_ST_CASER:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    return net->u_.case_.ncase;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_stmt_case_expr(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_CASE:
	  case IVL_ST_CASER:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    assert(idx < net->u_.case_.ncase);
	    return net->u_.case_.case_ex[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_statement_t ivl_stmt_case_stmt(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_CASE:
	  case IVL_ST_CASER:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    assert(idx < net->u_.case_.ncase);
	    return net->u_.case_.case_st + idx;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_stmt_cond_expr(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN_NB:
	    return net->u_.assign_.count;

	  case IVL_ST_CONDIT:
	    return net->u_.condit_.cond_;

	  case IVL_ST_CASE:
	  case IVL_ST_CASER:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    return net->u_.case_.cond;

	  case IVL_ST_REPEAT:
	  case IVL_ST_WHILE:
	    return net->u_.while_.cond_;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_statement_t ivl_stmt_cond_false(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_CONDIT);
      if (net->u_.condit_.stmt_[1].type_ == IVL_ST_NONE)
	    return 0;
      else
	    return net->u_.condit_.stmt_ + 1;
}

extern "C" ivl_statement_t ivl_stmt_cond_true(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_CONDIT);
      if (net->u_.condit_.stmt_[0].type_ == IVL_ST_NONE)
	    return 0;
      else
	    return net->u_.condit_.stmt_ + 0;
}

extern "C" ivl_expr_t ivl_stmt_delay_expr(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	    return net->u_.assign_.delay;

	  case IVL_ST_DELAYX:
	    return net->u_.delayx_.expr;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" uint64_t ivl_stmt_delay_val(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_DELAY);
      return net->u_.delay_.value;
}

extern "C" unsigned ivl_stmt_nevent(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN_NB:
	    return net->u_.assign_.nevent;

	  case IVL_ST_WAIT:
	    return net->u_.wait_.nevent;

	  case IVL_ST_TRIGGER:
	    return 1;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_event_t ivl_stmt_events(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN_NB:
	    assert(idx < net->u_.assign_.nevent);
	    if (net->u_.assign_.nevent == 1)
		  return net->u_.assign_.event;
	    else
		  return net->u_.assign_.events[idx];

	  case IVL_ST_WAIT:
	    assert(idx < net->u_.wait_.nevent);
	    if (net->u_.wait_.nevent == 1)
		  return net->u_.wait_.event;
	    else
		  return net->u_.wait_.events[idx];

	  case IVL_ST_TRIGGER:
	    assert(idx == 0);
	    return net->u_.wait_.event;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_expr_t ivl_stmt_lexp(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_CONTRIB:
	    return net->u_.contrib_.lval;
	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_lval_t ivl_stmt_lval(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	  case IVL_ST_CASSIGN:
	  case IVL_ST_DEASSIGN:
	  case IVL_ST_FORCE:
	  case IVL_ST_RELEASE:
	    assert(idx < net->u_.assign_.lvals_);
	    return net->u_.assign_.lval_ + idx;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" unsigned ivl_stmt_lvals(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	  case IVL_ST_CASSIGN:
	  case IVL_ST_DEASSIGN:
	  case IVL_ST_FORCE:
	  case IVL_ST_RELEASE:
	    return net->u_.assign_.lvals_;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" unsigned ivl_stmt_lwidth(ivl_statement_t net)
{
      assert((net->type_ == IVL_ST_ASSIGN)
	     || (net->type_ == IVL_ST_ASSIGN_NB)
	     || (net->type_ == IVL_ST_CASSIGN)
	     || (net->type_ == IVL_ST_DEASSIGN)
	     || (net->type_ == IVL_ST_FORCE)
	     || (net->type_ == IVL_ST_RELEASE));

      unsigned sum = 0;

      unsigned nlvals;
      struct ivl_lval_s*lvals;
      nlvals = net->u_.assign_.lvals_;
      lvals  = net->u_.assign_.lval_;

      for (unsigned idx = 0 ;  idx < nlvals ;  idx += 1) {
	    ivl_lval_t cur = lvals + idx;
	    switch(cur->type_) {
		case IVL_LVAL_MUX:
		  sum += 1;
		  break;
		case IVL_LVAL_REG:
		case IVL_LVAL_ARR:
		  sum += ivl_lval_width(cur);
		  break;
		default:
		  assert(0);
	    }
      }

      return sum;
}

extern "C" const char* ivl_stmt_name(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_STASK:
	    return net->u_.stask_.name_;
	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_expr_t ivl_stmt_parm(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_STASK:
	    assert(idx < net->u_.stask_.nparm_);
	    return net->u_.stask_.parms_[idx];

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" unsigned ivl_stmt_parm_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_STASK:
	    return net->u_.stask_.nparm_;
	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_expr_t ivl_stmt_rval(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	  case IVL_ST_CASSIGN:
	  case IVL_ST_FORCE:
	    return net->u_.assign_.rval_;
	  case IVL_ST_CONTRIB:
	    return net->u_.contrib_.rval;
	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_statement_t ivl_stmt_sub_stmt(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_DELAY:
	    return net->u_.delay_.stmt_;
	  case IVL_ST_DELAYX:
	    return net->u_.delayx_.stmt_;
	  case IVL_ST_FOREVER:
	    return net->u_.forever_.stmt_;
	  case IVL_ST_WAIT:
	    return net->u_.wait_.stmt_;
	  case IVL_ST_REPEAT:
	  case IVL_ST_WHILE:
	    return net->u_.while_.stmt_;
	  default:
	    assert(0);
      }

      return 0;
}

extern "C" const char*ivl_switch_basename(ivl_switch_t net)
{
      return net->name;
}

extern "C" ivl_scope_t ivl_switch_scope(ivl_switch_t net)
{
      return net->scope;
}

extern "C" ivl_switch_type_t ivl_switch_type(ivl_switch_t net)
{
      return net->type;
}

extern "C" ivl_nexus_t ivl_switch_a(ivl_switch_t net)
{
      return net->pins[0];
}

extern "C" ivl_nexus_t ivl_switch_b(ivl_switch_t net)
{
      return net->pins[1];
}

extern "C" ivl_nexus_t ivl_switch_enable(ivl_switch_t net)
{
      return net->pins[2];
}

extern "C" unsigned ivl_switch_width(ivl_switch_t net)
{
      return net->width;
}

extern "C" unsigned ivl_switch_part(ivl_switch_t net)
{
      return net->part;
}

extern "C" unsigned ivl_switch_offset(ivl_switch_t net)
{
      return net->offset;
}

extern "C" const char* ivl_switch_file(ivl_switch_t net)
{
      return net->file;
}

extern "C" ivl_island_t ivl_switch_island(ivl_switch_t net)
{
      return net->island;
}

extern "C" unsigned ivl_switch_lineno(ivl_switch_t net)
{
      return net->lineno;
}
