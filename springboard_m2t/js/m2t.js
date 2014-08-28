(function ($) {
  Drupal.behaviors.m2t = {
    attach: function (context, settings) {

      var addressFields = 'input[name$="address]"], input[name*="city"], select[name*="state"], input[name*="zip"]';

      $('form.geocode-form').on('blur', addressFields, function(){
        parseAddressFields();
      });

      $('form.geocode-form').on('change', 'select[name*="state"]', function(){
        parseAddressFields();
      });

      function parseAddressFields() {
        var address = '';
        $(addressFields,'form.geocode-form').each(function(){
           if(this.value === '') {
            return false;
           }
           else {
            address += this.value + ', ';
           }
           if(this.name == 'submitted[sbp_zip]' && this.value !== '') {
             lookupGeo(address);
           }
        });
      }

      function lookupGeo(address) {
        console.log(address);
        activeGeocoder = Drupal.settings.springboard_m2t.geocoder;
        if (activeGeocoder == 'google') {
          var geocoder = new google.maps.Geocoder();
          geocoder.geocode({'address': address}, function (results, status) {
            if (status == google.maps.GeocoderStatus.OK) {
              var latitude = results[0].geometry.location.lat();
              var longitude = results[0].geometry.location.lng();
              console.log(latitude + ' ' + longitude);

              $.ajax({
                type: "POST",
                url: '/sunlightlookup',
                data: {coords: [latitude, longitude]},
                dataType: 'json',
                success: function(data) {
                  console.log(data);
                  $('form.geocode-form').prepend(data.data);
                },

                error: function(xhr, textStatus, error){
                  console.log(error);
                }
              });

            } else {
              console.log(status);
            }
          });
          return false;
        }
        else {
          if (activeGeocoder == 'census') {
            url = '/censuslookup';
          }
          else {
            url = '/geocodiolookup';
          }
          $.ajax({
            type: "POST",
            url: url,
            data: {onelineaddress: address},
            dataType: 'json',
            success: function(data) {
              console.log(data);
              $('form.geocode-form').prepend(data.data);
            },

            error: function(xhr, textStatus, error){
              console.log(error);
            }
          });
        }
      }
    }
  };
})(jQuery);