// Error.h

#ifndef _JUST_PEER_ERROR_H_
#define _JUST_PEER_ERROR_H_

namespace just
{
    namespace peer
    {

        namespace error {

            enum errors
            {
                vod_success =0,
                start_failed, 
                bad_xml_format,
                write_mp4_head_error,
                bad_file_format
            };

            namespace detail {

                class vod_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const
                    {
                        return "peer";
                    }

                    std::string message(int value) const
                    {
                        switch (value) 
                        {
                        case vod_success:
                            return "operator success";
                        case start_failed:
                            return "peer start faield";
                        case bad_xml_format:
                            return "bad xml format";
                        case write_mp4_head_error:
                            return "save mp4 head failed";
                        case bad_file_format:
                            return "bad tmp file";
                        default:
                            return "peer other error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::vod_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace peer_error

    } // namespace peer
} // namespace just

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<just::peer::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using just::peer::error::make_error_code;
#endif

    }
}

#endif // _JUST_peer_ERROR_H_
