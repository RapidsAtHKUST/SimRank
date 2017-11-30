/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.giraph.utils;

import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Helper methods to get type arguments to generic classes.  Courtesy of
 * Ian Robertson (overstock.com).  Make sure to use with abstract
 * generic classes, not interfaces.
 */
public class ReflectionUtils {
  /**
   * Do not instantiate.
   */
  private ReflectionUtils() { }

  /**
   * Get the underlying class for a type, or null if the type is
   * a variable type.
   *
   * @param type the type
   * @return the underlying class
   */
  public static Class<?> getClass(Type type) {
    if (type instanceof Class) {
      return (Class<?>) type;
    } else if (type instanceof ParameterizedType) {
      return getClass(((ParameterizedType) type).getRawType());
    } else if (type instanceof GenericArrayType) {
      Type componentType =
          ((GenericArrayType) type).getGenericComponentType();
      Class<?> componentClass = getClass(componentType);
      if (componentClass != null) {
        return Array.newInstance(componentClass, 0).getClass();
      } else {
        return null;
      }
    } else {
      return null;
    }
  }

  /**
   * Get the actual type arguments a child class has used to extend a
   * generic base class.
   *
   * @param <T> Type to evaluate.
   * @param baseClass the base class
   * @param childClass the child class
   * @return a list of the raw classes for the actual type arguments.
   */
  public static <T> List<Class<?>> getTypeArguments(
      Class<T> baseClass, Class<? extends T> childClass) {
    Map<Type, Type> resolvedTypes = new HashMap<Type, Type>();
    Type type = childClass;
    // start walking up the inheritance hierarchy until we hit baseClass
    while (! getClass(type).equals(baseClass)) {
      if (type instanceof Class) {
        Type newType = ((Class<?>) type).getGenericSuperclass();
        if (newType == null) {
          // we have reached an interface, so we stop here
          break;
        } else {
          // there is no useful information for us in raw types,
          // so just keep going.
          type = newType;
        }

      } else {
        ParameterizedType parameterizedType = (ParameterizedType) type;
        Class<?> rawType = (Class<?>) parameterizedType.getRawType();

        Type[] actualTypeArguments =
            parameterizedType.getActualTypeArguments();
        TypeVariable<?>[] typeParameters = rawType.getTypeParameters();
        for (int i = 0; i < actualTypeArguments.length; i++) {
          resolvedTypes.put(typeParameters[i],
              actualTypeArguments[i]);
        }

        if (!rawType.equals(baseClass)) {
          type = rawType.getGenericSuperclass();
        }
      }
    }

    // finally, for each actual type argument provided to baseClass,
    // determine (if possible) the raw class for that type argument.
    Type[] actualTypeArguments;
    if (type instanceof Class) {
      actualTypeArguments = ((Class<?>) type).getTypeParameters();
    } else {
      actualTypeArguments =
          ((ParameterizedType) type).getActualTypeArguments();
    }
    List<Class<?>> typeArgumentsAsClasses = new ArrayList<Class<?>>();
    // resolve types by chasing down type variables.
    for (Type baseType: actualTypeArguments) {
      while (resolvedTypes.containsKey(baseType)) {
        baseType = resolvedTypes.get(baseType);
      }
      typeArgumentsAsClasses.add(getClass(baseType));
    }
    return typeArgumentsAsClasses;
  }

  /**
   * Try to directly set a (possibly private) field on an Object.
   *
   * @param target Target to set the field on.
   * @param fieldname Name of field.
   * @param value Value to set on target.
   */
  public static void setField(Object target, String fieldname, Object value)
    throws NoSuchFieldException, IllegalAccessException {
    Field field = findDeclaredField(target.getClass(), fieldname);
    field.setAccessible(true);
    field.set(target, value);
  }

  /**
   * Find a declared field in a class or one of its super classes
   *
   * @param inClass Class to search for declared field.
   * @param fieldname Field name to search for
   * @return Field or will throw.
   * @throws NoSuchFieldException When field not found.
   */
  private static Field findDeclaredField(Class<?> inClass, String fieldname)
    throws NoSuchFieldException {
    while (!Object.class.equals(inClass)) {
      for (Field field : inClass.getDeclaredFields()) {
        if (field.getName().equalsIgnoreCase(fieldname)) {
          return field;
        }
      }
      inClass = inClass.getSuperclass();
    }
    throw new NoSuchFieldException();
  }

  /**
   * Instantiate classes that are ImmutableClassesGiraphConfigurable
   *
   * @param theClass Class to instantiate
   * @param configuration Giraph configuration, may be null
   * @param <T> Type to instantiate
   * @return Newly instantiated object with configuration set if possible
   */
  @SuppressWarnings("unchecked")
  public static <T> T newInstance(
      Class<T> theClass,
      ImmutableClassesGiraphConfiguration configuration) {
    T result = null;
    try {
      result = theClass.newInstance();
    } catch (InstantiationException e) {
      throw new IllegalStateException(
          "newInstance: Couldn't instantiate " + theClass.getName(), e);
    } catch (IllegalAccessException e) {
      throw new IllegalStateException(
          "newInstance: Illegal access " + theClass.getName(), e);
    }
    if (configuration != null) {
      configuration.configureIfPossible(result);
    }
    return result;
  }
}
