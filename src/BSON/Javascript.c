/*
 * Copyright 2014-present MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <php.h>
#include <Zend/zend_interfaces.h>
#include <ext/standard/php_var.h>
#include <zend_smart_str.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "phongo_compat.h"
#include "php_phongo.h"
#include "php_bson.h"

zend_class_entry* php_phongo_javascript_ce;

/* Initialize the object and return whether it was successful. An exception will
 * be thrown on error. */
static bool php_phongo_javascript_init(php_phongo_javascript_t* intern, const char* code, size_t code_len, zval* scope) /* {{{ */
{
	if (scope && Z_TYPE_P(scope) != IS_OBJECT && Z_TYPE_P(scope) != IS_ARRAY && Z_TYPE_P(scope) != IS_NULL) {
		phongo_throw_exception(PHONGO_ERROR_INVALID_ARGUMENT, "Expected scope to be array or object, %s given", zend_get_type_by_const(Z_TYPE_P(scope)));
		return false;
	}

	if (strlen(code) != (size_t) code_len) {
		phongo_throw_exception(PHONGO_ERROR_INVALID_ARGUMENT, "Code cannot contain null bytes");
		return false;
	}

	intern->code     = estrndup(code, code_len);
	intern->code_len = code_len;

	if (scope && (Z_TYPE_P(scope) == IS_OBJECT || Z_TYPE_P(scope) == IS_ARRAY)) {
		intern->scope = bson_new();
		php_phongo_zval_to_bson(scope, PHONGO_BSON_NONE, intern->scope, NULL);
	} else {
		intern->scope = NULL;
	}

	return true;
} /* }}} */

/* Initialize the object from a HashTable and return whether it was successful.
 * An exception will be thrown on error. */
static bool php_phongo_javascript_init_from_hash(php_phongo_javascript_t* intern, HashTable* props) /* {{{ */
{
	zval *code, *scope;

	if ((code = zend_hash_str_find(props, "code", sizeof("code") - 1)) && Z_TYPE_P(code) == IS_STRING) {
		scope = zend_hash_str_find(props, "scope", sizeof("scope") - 1);

		return php_phongo_javascript_init(intern, Z_STRVAL_P(code), Z_STRLEN_P(code), scope);
	}

	phongo_throw_exception(PHONGO_ERROR_INVALID_ARGUMENT, "%s initialization requires \"code\" string field", ZSTR_VAL(php_phongo_javascript_ce->name));
	return false;
} /* }}} */

HashTable* php_phongo_javascript_get_properties_hash(phongo_compat_object_handler_type* object, bool is_temp) /* {{{ */
{
	php_phongo_javascript_t* intern;
	HashTable*               props;

	intern = Z_OBJ_JAVASCRIPT(PHONGO_COMPAT_GET_OBJ(object));

	PHONGO_GET_PROPERTY_HASH_INIT_PROPS(is_temp, intern, props, 2);

	if (!intern->code) {
		return props;
	}

	{
		zval code;

		ZVAL_STRING(&code, intern->code);
		zend_hash_str_update(props, "code", sizeof("code") - 1, &code);

		if (intern->scope) {
			php_phongo_bson_state state;

			PHONGO_BSON_INIT_STATE(state);
			if (!php_phongo_bson_to_zval_ex(bson_get_data(intern->scope), intern->scope->len, &state)) {
				zval_ptr_dtor(&state.zchild);
				goto failure;
			}

			zend_hash_str_update(props, "scope", sizeof("scope") - 1, &state.zchild);
		} else {
			zval scope;

			ZVAL_NULL(&scope);
			zend_hash_str_update(props, "scope", sizeof("scope") - 1, &scope);
		}
	}

	return props;

failure:
	PHONGO_GET_PROPERTY_HASH_FREE_PROPS(is_temp, props);
	return NULL;
} /* }}} */

/* {{{ proto void MongoDB\BSON\Javascript::__construct(string $code[, array|object $scope])
   Construct a new BSON Javascript type. The scope is a document mapping
   identifiers and values, representing the scope in which the code string will
   be evaluated. Note that this type cannot be represented as Extended JSON. */
static PHP_METHOD(Javascript, __construct)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;
	char*                    code;
	size_t                   code_len;
	zval*                    scope = NULL;

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|A!", &code, &code_len, &scope) == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	php_phongo_javascript_init(intern, code, code_len, scope);
} /* }}} */

/* {{{ proto MongoDB\BSON\Javascript MongoDB\BSON\Javascript::__set_state(array $properties)
*/
static PHP_METHOD(Javascript, __set_state)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;
	HashTable*               props;
	zval*                    array;

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &array) == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	object_init_ex(return_value, php_phongo_javascript_ce);

	intern = Z_JAVASCRIPT_OBJ_P(return_value);
	props  = Z_ARRVAL_P(array);

	php_phongo_javascript_init_from_hash(intern, props);
} /* }}} */

/* {{{ proto string MongoDB\BSON\Javascript::__toString()
   Return the Javascript's code string. */
static PHP_METHOD(Javascript, __toString)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters_none() == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	RETURN_STRINGL(intern->code, intern->code_len);
} /* }}} */

/* {{{ proto string MongoDB\BSON\Javascript::getCode()
*/
static PHP_METHOD(Javascript, getCode)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters_none() == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	RETURN_STRINGL(intern->code, intern->code_len);
} /* }}} */

/* {{{ proto object|null MongoDB\BSON\Javascript::getScope()
*/
static PHP_METHOD(Javascript, getScope)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters_none() == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	if (!intern->scope) {
		RETURN_NULL();
	}

	if (intern->scope->len) {
		php_phongo_bson_state state;

		PHONGO_BSON_INIT_STATE(state);

		if (!php_phongo_bson_to_zval_ex(bson_get_data(intern->scope), intern->scope->len, &state)) {
			zval_ptr_dtor(&state.zchild);
			return;
		}

		RETURN_ZVAL(&state.zchild, 0, 1);
	} else {
		RETURN_NULL();
	}
} /* }}} */

/* {{{ proto array MongoDB\BSON\Javascript::jsonSerialize()
*/
static PHP_METHOD(Javascript, jsonSerialize)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters_none() == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	array_init_size(return_value, 2);
	ADD_ASSOC_STRINGL(return_value, "$code", intern->code, intern->code_len);

	if (intern->scope && intern->scope->len) {
		php_phongo_bson_state state;

		PHONGO_BSON_INIT_STATE(state);
		if (!php_phongo_bson_to_zval_ex(bson_get_data(intern->scope), intern->scope->len, &state)) {
			zval_ptr_dtor(&state.zchild);
			return;
		}

		ADD_ASSOC_ZVAL_EX(return_value, "$scope", &state.zchild);
	}
} /* }}} */

/* {{{ proto string MongoDB\BSON\Javascript::serialize()
*/
static PHP_METHOD(Javascript, serialize)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;
	zval                     retval;
	php_phongo_bson_state    state;
	php_serialize_data_t     var_hash;
	smart_str                buf = { 0 };

	PHONGO_BSON_INIT_STATE(state);

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters_none() == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	if (intern->scope && intern->scope->len) {
		if (!php_phongo_bson_to_zval_ex(bson_get_data(intern->scope), intern->scope->len, &state)) {
			zval_ptr_dtor(&state.zchild);
			return;
		}
	} else {
		ZVAL_NULL(&state.zchild);
	}

	array_init_size(&retval, 2);
	ADD_ASSOC_STRINGL(&retval, "code", intern->code, intern->code_len);
	ADD_ASSOC_ZVAL(&retval, "scope", &state.zchild);

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&buf, &retval, &var_hash);
	smart_str_0(&buf);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	PHONGO_RETVAL_SMART_STR(buf);

	smart_str_free(&buf);
	zval_ptr_dtor(&retval);
} /* }}} */

/* {{{ proto void MongoDB\BSON\Javascript::unserialize(string $serialized)
*/
static PHP_METHOD(Javascript, unserialize)
{
	zend_error_handling      error_handling;
	php_phongo_javascript_t* intern;
	char*                    serialized;
	size_t                   serialized_len;
	zval                     props;
	php_unserialize_data_t   var_hash;

	intern = Z_JAVASCRIPT_OBJ_P(getThis());

	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &serialized, &serialized_len) == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}
	zend_restore_error_handling(&error_handling);

	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	if (!php_var_unserialize(&props, (const unsigned char**) &serialized, (unsigned char*) serialized + serialized_len, &var_hash)) {
		zval_ptr_dtor(&props);
		phongo_throw_exception(PHONGO_ERROR_UNEXPECTED_VALUE, "%s unserialization failed", ZSTR_VAL(php_phongo_javascript_ce->name));

		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		return;
	}
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

	php_phongo_javascript_init_from_hash(intern, HASH_OF(&props));
	zval_ptr_dtor(&props);
} /* }}} */

/* {{{ proto array MongoDB\Driver\Javascript::__serialize()
*/
static PHP_METHOD(Javascript, __serialize)
{
	PHONGO_PARSE_PARAMETERS_NONE();

	RETURN_ARR(php_phongo_javascript_get_properties_hash(PHONGO_COMPAT_OBJ_P(getThis()), true));
} /* }}} */

/* {{{ proto void MongoDB\Driver\Javascript::__unserialize(array $data)
*/
static PHP_METHOD(Javascript, __unserialize)
{
	zval* data;

	PHONGO_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_ARRAY(data)
	PHONGO_PARSE_PARAMETERS_END();

	php_phongo_javascript_init_from_hash(Z_JAVASCRIPT_OBJ_P(getThis()), Z_ARRVAL_P(data));
} /* }}} */

/* {{{ MongoDB\BSON\Javascript function entries */
/* clang-format off */
ZEND_BEGIN_ARG_INFO_EX(ai_Javascript___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, javascript)
	ZEND_ARG_INFO(0, scope)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Javascript___set_state, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, properties, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Javascript___unserialize, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, data, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(ai_Javascript_jsonSerialize, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Javascript_unserialize, 0, 0, 1)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Javascript_void, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_phongo_javascript_me[] = {
	PHP_ME(Javascript, __construct, ai_Javascript___construct, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, __serialize, ai_Javascript_void, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, __set_state, ai_Javascript___set_state, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Javascript, __toString, ai_Javascript_void, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, __unserialize, ai_Javascript___unserialize, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, jsonSerialize, ai_Javascript_jsonSerialize, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, serialize, ai_Javascript_void, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, unserialize, ai_Javascript_unserialize, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, getCode, ai_Javascript_void, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(Javascript, getScope, ai_Javascript_void, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_FE_END
};
/* clang-format on */
/* }}} */

/* {{{ MongoDB\BSON\Javascript object handlers */
static zend_object_handlers php_phongo_handler_javascript;

static void php_phongo_javascript_free_object(zend_object* object) /* {{{ */
{
	php_phongo_javascript_t* intern = Z_OBJ_JAVASCRIPT(object);

	zend_object_std_dtor(&intern->std);

	if (intern->code) {
		efree(intern->code);
	}
	if (intern->scope) {
		bson_destroy(intern->scope);
		intern->scope = NULL;
	}

	if (intern->properties) {
		zend_hash_destroy(intern->properties);
		FREE_HASHTABLE(intern->properties);
	}
} /* }}} */

zend_object* php_phongo_javascript_create_object(zend_class_entry* class_type) /* {{{ */
{
	php_phongo_javascript_t* intern = NULL;

	intern = PHONGO_ALLOC_OBJECT_T(php_phongo_javascript_t, class_type);
	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_phongo_handler_javascript;

	return &intern->std;
} /* }}} */

static zend_object* php_phongo_javascript_clone_object(phongo_compat_object_handler_type* object) /* {{{ */
{
	php_phongo_javascript_t* intern;
	php_phongo_javascript_t* new_intern;
	zend_object*             new_object;

	intern     = Z_OBJ_JAVASCRIPT(PHONGO_COMPAT_GET_OBJ(object));
	new_object = php_phongo_javascript_create_object(PHONGO_COMPAT_GET_OBJ(object)->ce);

	new_intern = Z_OBJ_JAVASCRIPT(new_object);
	zend_objects_clone_members(&new_intern->std, &intern->std);

	php_phongo_javascript_init(new_intern, intern->code, intern->code_len, NULL);
	new_intern->scope = bson_copy(intern->scope);

	return new_object;
} /* }}} */

static int php_phongo_javascript_compare_objects(zval* o1, zval* o2) /* {{{ */
{
	php_phongo_javascript_t *intern1, *intern2;

	ZEND_COMPARE_OBJECTS_FALLBACK(o1, o2);

	intern1 = Z_JAVASCRIPT_OBJ_P(o1);
	intern2 = Z_JAVASCRIPT_OBJ_P(o2);

	/* Do not consider the scope document for comparisons */
	return strcmp(intern1->code, intern2->code);
} /* }}} */

static HashTable* php_phongo_javascript_get_debug_info(phongo_compat_object_handler_type* object, int* is_temp) /* {{{ */
{
	*is_temp = 1;
	return php_phongo_javascript_get_properties_hash(object, true);
} /* }}} */

static HashTable* php_phongo_javascript_get_properties(phongo_compat_object_handler_type* object) /* {{{ */
{
	return php_phongo_javascript_get_properties_hash(object, false);
} /* }}} */
/* }}} */

void php_phongo_javascript_init_ce(INIT_FUNC_ARGS) /* {{{ */
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "MongoDB\\BSON", "Javascript", php_phongo_javascript_me);
	php_phongo_javascript_ce                = zend_register_internal_class(&ce);
	php_phongo_javascript_ce->create_object = php_phongo_javascript_create_object;
	PHONGO_CE_FINAL(php_phongo_javascript_ce);

	zend_class_implements(php_phongo_javascript_ce, 1, php_phongo_javascript_interface_ce);
	zend_class_implements(php_phongo_javascript_ce, 1, php_phongo_json_serializable_ce);
	zend_class_implements(php_phongo_javascript_ce, 1, php_phongo_type_ce);
	zend_class_implements(php_phongo_javascript_ce, 1, zend_ce_serializable);

	memcpy(&php_phongo_handler_javascript, phongo_get_std_object_handlers(), sizeof(zend_object_handlers));
	PHONGO_COMPAT_SET_COMPARE_OBJECTS_HANDLER(javascript);
	php_phongo_handler_javascript.clone_obj      = php_phongo_javascript_clone_object;
	php_phongo_handler_javascript.get_debug_info = php_phongo_javascript_get_debug_info;
	php_phongo_handler_javascript.get_properties = php_phongo_javascript_get_properties;
	php_phongo_handler_javascript.free_obj       = php_phongo_javascript_free_object;
	php_phongo_handler_javascript.offset         = XtOffsetOf(php_phongo_javascript_t, std);
} /* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
