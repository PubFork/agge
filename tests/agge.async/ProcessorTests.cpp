#include <agge.async/processor.h>

#include <tests/common/mt.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			class work1
			{
			public:
				work1(int value_ = 0)
					: value(value_), tid(this_thread_id())
				{	}

				void operator ()()
				{	tid = this_thread_id();	}

			public:
				int value;
				thread_id tid;
			};

			bool dummy_queue_handler(int)
			{	return true;	}


			template <typename T>
			class consumer
			{
			public:
				consumer(T &target)
					: _target(&target)
				{	}

				void operator ()(T &value) const
				{	*_target = value;	}

			private:
				T *_target;
			};

			template <typename T>
			consumer<T> consume_into(T &output)
			{	return consumer<T>(output);	}

			class signaler : noncopyable
			{
			public:
				signaler(hybrid_event &e)
					: _e(e)
				{	}

				void operator ()(int n) const
				{
					if (!n)
						_e.signal();
				}

			private:
				hybrid_event & _e;
			};

			class waiter : noncopyable
			{
			public:
				waiter(hybrid_event &e)
					: _e(e)
				{	}

				bool operator ()(int n) const
				{
					if (!n)
						_e.wait();
					return true;
				}

			private:
				hybrid_event & _e;
			};
		}

		begin_test_suite( ProcessorTests )
			test( ConstructedProcessorCanBeDestroyed )
			{
				// INIT / ACT
				processor<work1>::input_queue_t input;
				processor<work1>::output_queue_t output;
				hybrid_event has_input, output_ready;
				auto_ptr< processor<work1> > p(new processor<work1>(input, has_input, output, output_ready));

				// ACT / ASSERT (must not hang)
				p.reset();
			}

			test( ItemGetsProcessedInADedicatedThread )
			{
				// INIT
				processor<work1>::input_queue_t input;
				processor<work1>::output_queue_t output;
				hybrid_event has_input, output_ready;
				processor<work1> p(input, has_input, output, output_ready);

				// ACT / ASSERT (must not hang in wait())
				input.produce(work1(1), signaler(has_input));

				// ASSERT
				work1 ovalue;

				output.consume(consume_into(ovalue), waiter(output_ready));

				assert_not_equal(this_thread_id(), ovalue.tid);
			}


			test( MultipleWorksCanBeSubmittedForProcessing )
			{
				// INIT
				processor<work1>::input_queue_t input;
				processor<work1>::output_queue_t output;
				hybrid_event has_input, output_ready;
				processor<work1> p(input, has_input, output, output_ready);

				// ACT / ASSERT
				input.produce(work1(7), signaler(has_input));

				// ACT / ASSERT
				work1 ovalue;

				output.consume(consume_into(ovalue), waiter(output_ready));
				assert_equal(7, ovalue.value);

				// ACT / ASSERT
				input.produce(work1(13), signaler(has_input));

				// ACT / ASSERT
				output.consume(consume_into(ovalue), waiter(output_ready));
				assert_equal(13, ovalue.value);

				// ACT / ASSERT
				input.produce(work1(11), signaler(has_input));

				// ACT / ASSERT
				output.consume(consume_into(ovalue), waiter(output_ready));
				assert_equal(11, ovalue.value);
			}


			test( BatchOfWorksCanBeSubmittedForProcessing )
			{
				// INIT
				processor<work1>::input_queue_t input;
				processor<work1>::output_queue_t output;
				hybrid_event has_input, output_ready;
				processor<work1> p(input, has_input, output, output_ready);

				// ACT / ASSERT
				input.produce(work1(7), signaler(has_input));
				input.produce(work1(13), signaler(has_input));
				input.produce(work1(5), signaler(has_input));

				// ACT / ASSERT
				work1 ovalue;

				output.consume(consume_into(ovalue), waiter(output_ready));
				assert_equal(7, ovalue.value);

				// ACT / ASSERT
				output.consume(consume_into(ovalue), waiter(output_ready));
				assert_equal(13, ovalue.value);

				// ACT / ASSERT
				output.consume(consume_into(ovalue), waiter(output_ready));
				assert_equal(5, ovalue.value);
			}
		end_test_suite
	}
}
