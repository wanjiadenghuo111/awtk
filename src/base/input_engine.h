﻿/**
 * File:   input_engine.h
 * Author: AWTK Develop Team
 * Brief:  input method engine
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-06-23 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_INPUT_ENGINE_H
#define TK_INPUT_ENGINE_H

#include "base/str.h"

BEGIN_C_DECLS

struct _input_engine_t;
typedef struct _input_engine_t input_engine_t;

typedef ret_t (*input_engine_reset_input_t)(input_engine_t* engine);
typedef ret_t (*input_engine_input_t)(input_engine_t* engine, int key);

#define TK_IM_MAX_INPUT_CHARS 31
#define TK_IM_MAX_CANDIDATE_CHARS 127

/**
 * @class input_engine_t
 * @scriptable no
 * 输入法引擎接口。
 */
typedef struct _input_engine_t {
  /**
   * @property {str_t} keys
   * @private
   * 当前的按键组合。
   */
  str_t keys;
  /**
   * @property {char*} candidates
   * @private
   * 当前按键组合对应的候选字列表。
   */
  char candidates[TK_IM_MAX_CANDIDATE_CHARS + 1];
  /**
   * @property {uint32_t} candidates_nr
   * @private
   * 当前按键组合对应的候选字/词个数。
   */
  uint32_t candidates_nr;

  /*具体实现需要实现的函数*/
  input_engine_input_t input;
  input_engine_reset_input_t reset_input;
} input_engine_t;

/**
 * @method input_engine_create
 * @constructor
 * 创建输入法引擎对象。
 *
 * @return {input_engine_t*} 返回输入法引擎对象。
 */
input_engine_t* input_engine_create(void);

/**
 * @method input_engine_destroy
 * @deconstructor
 * 销毁输入法引擎对象。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_destroy(input_engine_t* engine);

/**
 * @method input_engine_reset_input
 * 清除所有输入的按键组合和候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_reset_input(input_engine_t* engine);

/**
 * @method input_engine_input
 * 输入新的按键，并更新候选字。
 * @param {input_engine_t*} engine 输入法引擎对象。
 * @param {int} key 键值。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t input_engine_input(input_engine_t* engine, int key);

END_C_DECLS

#endif /*TK_INPUT_ENGINE_H*/
