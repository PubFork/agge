#include <agge/rasterizer.h>

#include <agge/renderer.h>

#include "helpers.h"
#include "mocks.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

namespace agge
{
	namespace tests
	{
		namespace
		{
			template <typename T>
			class passthrough_clipper
			{
			public:
				typedef T coord_type;

			public:
				void move_to(T x, T y)
				{	_last_x = x, _last_y = y;	}

				template <typename LinesSinkT>
				void line_to(LinesSinkT &sink, T x, T y)
				{
					sink.line(_last_x, _last_y, x, y);
					move_to(x, y);
				}

			private:
				T _last_x, _last_y;
			};

			template <typename T, const mocks::coords_pair<T> * const coords>
			class emitting_clipper
			{
			public:
				typedef T coord_type;

			public:
				void move_to(T /*x*/, T /*y*/)
				{	}

				template <typename LinesSinkT>
				void line_to(LinesSinkT &sink, T /*x*/, T /*y*/)
				{
					sink.line(coords->x1, coords->y1, coords->x1, coords->y2);
					sink.line(coords->x1, coords->y2, coords->x2, coords->y2);
					sink.line(coords->x2, coords->y2, coords->x2, coords->y1);
					sink.line(coords->x2, coords->y1, coords->x1, coords->y1);
				}
			};

			template <int k1 = 1, int k2 = 1>
			struct scaling_r
			{
				static void scale1(real_t x, real_t y, real_t &cx, real_t &cy)
				{	cx = k1 * x, cy = k1 * y;	}

				static void scale2(real_t x1, real_t y1, real_t x2, real_t y2, int &cx1, int &cy1, int &cx2, int &cy2)
				{
					cx1 = iround(256.0f * k2 * x1), cy1 = iround(256.0f * k2 * y1);
					cx2 = iround(256.0f * k2 * x2), cy2 = iround(256.0f * k2 * y2);
				}
			};

			template <int k1 = 1, int k2 = 1>
			struct scaling_i
			{
				static void scale1(real_t x, real_t y, int &cx, int &cy)
				{	cx = iround(256.0f * k1 * x), cy = iround(256.0f * k1 * y);	}

				static void scale2(int x1, int y1, int x2, int y2, int &cx1, int &cy1, int &cx2, int &cy2)
				{	cx1 = k2 * x1, cy1 = k2 * y1, cx2 = k2 * x2, cy2 = k2 * y2;	}
			};

			extern const mocks::coords_pair<int> c_box1 = { 0x0080, 0x0A4, 0x238, 0x2C0 };
			extern const mocks::coords_pair<real_t> c_box2 = { 0.5f, 0.640625f, 2.21875f, 2.75f };
		}

		begin_test_suite( RasterizerTests )
			test( DrawingAFigureWillRenderItToABitmap )
			{
				// INIT
				rasterizer< passthrough_clipper<real_t> > r1;
				rasterizer< passthrough_clipper<int> > r2;
				mocks::bitmap<uint8_t, 1, 1> bitmap1(6, 5);
				mocks::bitmap<uint8_t, 1, 1> bitmap1i(6, 5);
				mocks::bitmap<uint16_t, 1> bitmap2(7, 6);
				renderer r;

				// ACT
				r1.move_to(3.1f, 0.3f);
				r1.line_to(-2.422f, 2.705f);
				r1.line_to(6.43f, 6.72f);
				r1.line_to(3.1f, 0.3f);
				r1.sort();
				r(bitmap1, 0, r1, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				r2.move_to(3.1f, 0.3f);
				r2.line_to(-2.422f, 2.705f);
				r2.line_to(6.43f, 6.72f);
				r2.line_to(3.1f, 0.3f);
				r2.sort();
				r(bitmap1i, 0, r2, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x00, 0x0e, 0x70, 0x31, 0x00, 0x00, 0x00,
					0x92, 0xf2, 0xff, 0xb8, 0x00, 0x00, 0x00,
					0xff, 0xff, 0xff, 0xff, 0x3d, 0x00, 0x00,
					0xf4, 0xff, 0xff, 0xff, 0xc2, 0x00, 0x00,
					0x12, 0x7b, 0xe7, 0xff, 0xff, 0x47, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap1.data);
				assert_equal(reference1, bitmap1i.data);

				// ACT
				r1.reset();
				r1.move_to(1.0f, 1.0f);
				r1.line_to(1.0f, 5.0f);
				r1.line_to(4.5f, 5.0f);
				r1.line_to(4.5f, 1.0f);
				r1.line_to(1.0f, 1.0f);
				r1.sort();
				r(bitmap2, 0, r1, mocks::blender<uint16_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint16_t reference2[] = {
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x01FF, 0x01FF, 0x01FF, 0x0180, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				};

				assert_equal(reference2, bitmap2.data);
			}


			test( ClosingAPolygonDrawsLineToTheLastMoveToPoint )
			{
				// INIT
				rasterizer< passthrough_clipper<real_t> > ras;
				mocks::bitmap<uint8_t> bitmap(5, 5);
				renderer r;

				// ACT
				ras.move_to(0.9f, 0.1f);
				ras.line_to(1.2f, 4.7f);
				ras.line_to(4.3f, 3.1f);
				ras.close_polygon();
				ras.sort();
				r(bitmap, 0, ras, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x0f, 0x5e, 0x00, 0x00, 0x00,
					0x04, 0xfe, 0x7d, 0x00, 0x00,
					0x00, 0xf2, 0xff, 0x9a, 0x03,
					0x00, 0xe1, 0xf2, 0x83, 0x0c,
					0x00, 0x69, 0x14, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap.data);

				// INIT
				bitmap.data.assign(bitmap.data.size(), 0);
				ras.reset();

				// ACT
				ras.move_to(1.2f, 2.1f);
				ras.line_to(1.2f, 4.7f);
				ras.line_to(4.3f, 3.1f);
				ras.close_polygon();
				ras.sort();
				r(bitmap, 0, ras, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference2[] = {
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x9d, 0x7b, 0x28, 0x00,
					0x00, 0xcd, 0xf2, 0x82, 0x09,
					0x00, 0x64, 0x14, 0x00, 0x00,
				};

				assert_equal(reference2, bitmap.data);
			}


			test( CoordinatesAreTranslatedAfterClipping )
			{
				// INIT
				mocks::bitmap<uint8_t> b1(3, 3), b2(7, 9), b3(3, 3), b4(5, 6);
				rasterizer< emitting_clipper<int, &c_box1>, scaling_i<0, 1> > r1;
				rasterizer< emitting_clipper<int, &c_box1>, scaling_i<0, 3> > r2;
				rasterizer< emitting_clipper<real_t, &c_box2>, scaling_r<0, 1> > r3;
				rasterizer< emitting_clipper<real_t, &c_box2>, scaling_r<0, 2> > r4;
				renderer r;

				// ACT
				r1.line_to(0.0f, 0.0f);
				r1.sort();
				r(b1, 0, r1, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				r2.line_to(0.0f, 0.0f);
				r2.sort();
				r(b2, 0, r2, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				r3.line_to(0.0f, 0.0f);
				r3.sort();
				r(b3, 0, r3, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				r4.line_to(0.0f, 0.0f);
				r4.sort();
				r(b4, 0, r4, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x2e, 0x5c, 0x14,
					0x80, 0xff, 0x38,
					0x60, 0xc0, 0x2a,
				};

				assert_equal(reference1, b1.data);
				assert_equal(reference1, b3.data);

				uint8_t reference2[] = {
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x0a, 0x14, 0x14, 0x14, 0x14, 0x0d,
					0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xa8,
					0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xa8,
					0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xa8,
					0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xa8,
					0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xa8,
					0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xa8,
					0x00, 0x20, 0x40, 0x40, 0x40, 0x40, 0x2a,
				};

				assert_equal(reference2, b2.data);

				uint8_t reference4[] = {
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0xB8, 0xB8, 0xB8, 0x50,
					0x00, 0xFF, 0xFF, 0xFF, 0x70,
					0x00, 0xFF, 0xFF, 0xFF, 0x70,
					0x00, 0xFF, 0xFF, 0xFF, 0x70,
					0x00, 0x80, 0x80, 0x80, 0x38,
				};

				assert_equal(reference4, b4.data);
			}


			test( CoordinatesAreTranslatedBeforeClipping )
			{
				// INIT
				mocks::bitmap<uint8_t> b1(5, 4), b2(9, 7), b3(13, 10);
				rasterizer< passthrough_clipper<int>, scaling_i<1, 1> > r1;
				rasterizer< passthrough_clipper<int>, scaling_i<2, 1> > r2;
				rasterizer< passthrough_clipper<real_t>, scaling_r<3, 1> > r3;
				renderer r;

				// ACT
				r1.move_to(1.0f, 1.0f),
					r1.line_to(1.0f, 3.0f),
					r1.line_to(4.0f, 3.0f),
					r1.line_to(4.0f, 1.0f),
					r1.close_polygon();
				r1.sort();

				r2.move_to(1.0f, 1.0f),
					r2.line_to(1.0f, 3.0f),
					r2.line_to(4.0f, 3.0f),
					r2.line_to(4.0f, 1.0f),
					r2.close_polygon();
				r2.sort();

				r3.move_to(1.0f, 1.0f),
					r3.line_to(1.0f, 3.0f),
					r3.line_to(4.0f, 3.0f),
					r3.line_to(4.0f, 1.0f),
					r3.close_polygon();
				r3.sort();

				r(b1, 0, r1, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());
				r(b2, 0, r2, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());
				r(b3, 0, r3, mocks::blender<uint8_t, uint8_t>(), mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0xff, 0xff, 0xff, 0x00,
					0x00, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference1, b1.data);

				uint8_t reference2[] = {
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference2, b2.data);

				uint8_t reference3[] = {
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference3, b3.data);
			}

		end_test_suite
	}
}