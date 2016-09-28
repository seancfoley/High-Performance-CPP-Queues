/*
 * Access.h
 *
 *  Created on: Nov 17, 2010
 *      Author: sfoley
 */

#ifndef ACCESS_H_
#define ACCESS_H_

namespace hpqueue {

/*
 * This is a conversion class which contains conversion member functions between a base class and its subclass.
 *
 * It is used to control which classes are allowed to access the data members in data classes.
 *
 * It permits the conversion from subclass to base class and vice versa, but only friends can use it.
 */
template <class C, class Base>
class Access {
	friend class SampleQueueEntryConsumer;

	C &object;

	Access(Base &obj) : object(static_cast<C &>(obj)) {}

	Access(C &object) : object(object) {}

	operator Base *() {
		return &object;
	}

	operator C &() {
		return object;
	}

	operator Base &() {
		return object;
	}
};

}

#endif /* ACCESS_H_ */
